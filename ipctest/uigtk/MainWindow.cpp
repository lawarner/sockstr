/*
   Copyright (C) 2012
   Andy Warner
   This file is part of the sockstr class library.

   The sockstr class library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The sockstr class library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the sockstr library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

// MainWindow.cpp
//

#include <fstream>
#include <iostream>
#include <sstream>

#include "MainWindow.h"
#include "HistoryList.h"
#include "BuiltinCommands.h"
#include "Field.h"
#include "Log.h"
#include "Parser.h"
#include "RunContext.h"
#include "TestBase.h"


using namespace ipctest::uigtk;


MainWindow::MainWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::Window(cobject)
    , builder_(builder)
    , context_(*new ipctest::RunContext)
    , testBase_(new ipctest::TestBase)
    , docModified_(false)
    , programQuitting_(false)
{
    initDialog();
}


MainWindow::~MainWindow()
{
    delete testBase_;
}


bool MainWindow::initDialog()
{
    Gtk::TreeModel::Row row;

    commandList_ = Glib::RefPtr<Gtk::ListStore>::
        cast_dynamic(builder_->get_object("command_list"));

    std::vector<std::string> builtIns = testBase_->builtinCommandNames();
    std::vector<std::string>::const_iterator it;
    for (it = builtIns.begin(); it != builtIns.end(); ++it)
    {
        row = *(commandList_->append());
        row[commandColumns_.colName_] = (*it);
    }

    Glib::RefPtr<Gtk::ListStore> histList = Glib::RefPtr<Gtk::ListStore>::
        cast_dynamic(builder_->get_object("history_list"));
    Gtk::TreeView* histView;
    builder_->get_widget("history_view", histView);
    historyList_ = new HistoryList(this, histList, histView);

    messageList_ = Glib::RefPtr<Gtk::ListStore>::
        cast_dynamic(builder_->get_object("message_list"));
    messageTableList_ = Glib::RefPtr<Gtk::ListStore>::
        cast_dynamic(builder_->get_object("messagetable_list"));

    Gtk::Entry* conn;
    builder_->get_widget("connectionUrl",  conn);
    conn->set_text("localhost");
    builder_->get_widget("connectionPort", conn);
    conn->set_text("4321");

    builder_->get_widget("input_grid", inputGrid_);

    builder_->get_widget("commands", commands_);
    commands_->pack_start(commandColumns_.colName_);

    builder_->get_widget("message", messageName_);

    builder_->get_widget("messagelist_view", messageListView_);
    messageListView_->append_column("Ordinal", mlColumns_.colOrdinal_);
    messageListView_->append_column("Message", mlColumns_.colName_);

    builder_->get_widget("messagetable_view", messageTableView_);
    fieldTypeColumn_.set_title("Type");
    fieldTypeColumn_.pack_start(fieldTypeRenderer_);
    fieldTypeColumn_.set_cell_data_func(fieldTypeRenderer_,
                                        sigc::mem_fun(*this, &MainWindow::onCellDataFieldType));

    messageTableView_->append_column("Field Name", mtlColumns_.colFieldName_);
    messageTableView_->append_column(fieldTypeColumn_);
    messageTableView_->append_column_editable("Value", mtlColumns_.colFieldValue_);

    builder_->get_widget("status_bar", statusBar_);
    pixConnected_ = Gdk::Pixbuf::create_from_file("images/connected.png");
    pixDisconnected_ = Gdk::Pixbuf::create_from_file("images/disconnected.png");
//    statusIcon_ = Gtk::StatusIcon::create(pixDisconnected_).operator->();
    statusIcon_ = new Gtk::Image(pixDisconnected_);
    statusIcon_->show();
    statusBar_->pack_end(*statusIcon_, Gtk::PACK_SHRINK);
    statusBar_->push("Ready (Offline)");

    // Connect signal handlers
    builder_->get_widget("connect_button", connectButton_);
    connectButton_->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onConnect));
    Gtk::Button* button;
    builder_->get_widget("execute_button", button);
    button->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onExecute));

    messageListView_->signal_row_activated()
        .connect(sigc::mem_fun(*this, &MainWindow::onMessageActivated));
    Glib::RefPtr<Gtk::TreeSelection> selection = messageListView_->get_selection();
    selection->signal_changed().connect(sigc::mem_fun(*this, &MainWindow::onMessageSelection));

    commands_->signal_changed()
        .connect(sigc::mem_fun(*this, &MainWindow::onCommandChanged));

    signal_unmap().connect(sigc::mem_fun(*this, &MainWindow::onQuit));

    Gtk::ImageMenuItem* menuit;
    builder_->get_widget("menuquit", menuit);
    menuit->signal_activate().connect(sigc::mem_fun(*this, &MainWindow::onQuit));
    builder_->get_widget("menusave", menuit);
    menuit->signal_activate().connect(sigc::mem_fun(*this, &MainWindow::onSave));
    builder_->get_widget("menusaveas", menuit);
    menuit->signal_activate().connect(sigc::mem_fun(*this, &MainWindow::onSaveAs));
    builder_->get_widget("menuopen", menuit);
    menuit->signal_activate().connect(sigc::mem_fun(*this, &MainWindow::onOpen));

    builder_->get_widget("menurun", menuit);
    menuit->signal_activate().connect(sigc::mem_fun(*this, &MainWindow::onRun));
    builder_->get_widget("menudemote", menuit);
    menuit->signal_activate().connect(sigc::mem_fun(*this, &MainWindow::onDemote));
    builder_->get_widget("menupromote", menuit);
    menuit->signal_activate().connect(sigc::mem_fun(*this, &MainWindow::onPromote));

//    log(new CommandComment("initDialog complete."));

    return true;
}


void MainWindow::log(ipctest::Command* cmd)
{
    std::cout << "LOG: " << cmd->toString() << std::endl;
    historyList_->add(cmd);
    testBase_->addCommand(cmd);
    docModified_ = true;

    std::string fstr(testBase_->getFileName());
    if (fstr.empty())
        fstr = "(No name)*";
    else
        fstr += "*";

    set_title(fstr);
}

void MainWindow::guiToParams(ipctest::Params* params)
{
    if (!params)
        return;

    const ParamMap& pm = params->getAllParams();
    ParamMap::const_iterator it;
    for (it = pm.begin(); it != pm.end(); ++it)
    {
        ParamValue* pv = it->second;
        Gtk::Entry* inEntry = pv->widget;
        if (inEntry)
        {
            std::string str = inEntry->get_text();
            std::cout << "+guiToParam=" << it->first << " widget=" << inEntry 
                      << "  value=" << str << std::endl;
            pv->strValue = str;
        }
    }

}


void MainWindow::paramsToGui(ipctest::Params* params)
{
    // First, clear any previous widgets
    while (!inputWidgets_.empty())
    {
        Gtk::Widget* wid = inputWidgets_.top();
        inputWidgets_.pop();
        if (wid)
            delete wid;
    }

    if (!params)
        return;

    Gtk::Label* endLabel;
    builder_->get_widget("endlabel", endLabel);
    const ParamMap& pm = params->getAllParams();
    ParamMap::const_iterator it;
    for (it = pm.begin(); it != pm.end(); ++it)
    {
        if (it->first[0] == '_')
            continue;
        Gtk::Label* inLabel = new Gtk::Label(it->first + ":");
        inLabel->show();
        inputGrid_->attach_next_to(*inLabel, *endLabel, Gtk::POS_BOTTOM, 1, 1);
        inputWidgets_.push(inLabel);
        Gtk::Entry* inEntry = new Gtk::Entry;
        inEntry->set_text(it->second->strValue);
        inEntry->show();
        inputGrid_->attach_next_to(*inEntry, *inLabel, Gtk::POS_RIGHT, 1, 1);
        inputWidgets_.push(inEntry);
        it->second->widget = inEntry;
//        params->setWidget(it->first, inEntry);
        std::cout << "+paramsToGui param=" << it->first << " widget=" << inEntry << std::endl;

        endLabel = inLabel;
    }

}


void MainWindow::setCommand(ipctest::Command* cmd)
{
    Glib::RefPtr<Gtk::TreeModel> cmdModel = commands_->get_model();
    Gtk::TreeModel::iterator iter = cmdModel->children().begin();

    testBase_->setWorkCommand(cmd);

    for ( ; iter != cmdModel->children().end(); ++iter)
    {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring cmdr = row[commandColumns_.colName_];
        if (cmdr.raw() == cmd->getName())
        {
            commands_->set_active(iter);
            break;
        }
    }

    // copy fields
    Message* msg = cmd->getMessage();
    if (msg)
    {
        if (!cmd->getData())
            cmd->setData(new char[msg->getSize()]);

//        messageList_->children();
        Gtk::TreeModel::iterator iter = messageList_->children().begin();

        for ( ; iter != messageList_->children().end(); ++iter)
        {
            Gtk::TreeModel::Row row = *iter;
            if (row[mlColumns_.colName_] == msg->getName())
            {
                Glib::RefPtr<Gtk::TreeSelection> selection = messageListView_->get_selection();
                selection->select(iter);
                break;
            }
        }

        std::vector<std::string> fldValues;
        int szdata = msg->unpackFields(static_cast<const char *>(cmd->getData()),
                                       fldValues);
        if (szdata)
        {
            context_.setFieldValues(fldValues);
            stringsToFields(fldValues);
        }
    }
}


bool MainWindow::setup(const std::string& defFilename, const std::string& testFilename)
{
    if (!testBase_->readIpcDefs(defFilename))
        return false;

    const ipctest::MessageList& ml = testBase_->messageList();
    ipctest::MessageList::const_iterator it;
    for (it = ml.begin(); it != ml.end(); ++it)
    {
        std::cout << "Message: " << (*it)->getName() << std::endl;
        Gtk::TreeModel::Row row = *(messageList_->append());
        row[mlColumns_.colOrdinal_] = (*it)->getOrdinal();
        row[mlColumns_.colName_] = (*it)->getName();
        row[mlColumns_.colMessage_] = (*it);
    }

    if (testBase_->deserialize(testFilename))
    {
        historyList_->clear();
        CommandList& commandList = testBase_->commandList();
        CommandIterator it = commandList.begin();
        for ( ; it != commandList.end(); ++it)
            historyList_->add(*it);
//        docModified_ = false;
    }

    return true;
}


void MainWindow::stringsToFields(const std::vector<std::string>& flds)
{
    std::vector<std::string>::const_iterator itstr = flds.begin();
    Gtk::TreeModel::iterator iter = messageTableList_->children().begin();

    for ( ; iter != messageTableList_->children().end() && itstr != flds.end();
          ++iter, ++itstr)
    {
        Gtk::TreeModel::Row row = *iter;
        std::string str = *itstr;
        row[mtlColumns_.colFieldValue_] = str;
        std::cout << "stringsToFields: " << str << std::endl;
    }
}


void MainWindow::onCellDataFieldType(Gtk::CellRenderer* renderer,
                                     const Gtk::TreeModel::iterator& iter)
{
    if (iter)
    {
        Glib::ustring val = "";
        Gtk::TreeModel::Row row = *iter;
        ipctest::Field* fld = row[mtlColumns_.colField_];
        if (fld)
        {
            val = fld->type().toString();
            if (fld->elements() > 1)
                val += Glib::ustring::compose("[%1]", fld->elements());
        }

        fieldTypeRenderer_.property_text() = val;
    }
}


void MainWindow::onConnect()
{
    if (!testBase_->isConnected())
    {
        Gtk::Entry* connUrl;
        Gtk::Entry* connPort;
        builder_->get_widget("connectionUrl",  connUrl);
        builder_->get_widget("connectionPort", connPort);
        std::string url = connUrl->get_text();
        url += ":" + connPort->get_text();
        std::cout << "Connect to url: " << url << "." << std::endl;

        //TODO: if url == ":" only, connect a server socket
        ipctest::CommandConnect* connect = new ipctest::CommandConnect(url);
        if (connect->execute(context_))
        {
            testBase_->setSocket(context_.getSocket());
            testBase_->setConnected(true);
            connectButton_->set_label("Disconnect");
            log(connect);

            statusBar_->push(connect->toString());
            statusIcon_->set(pixConnected_);
        }
    }
    else
    {
        ipctest::CommandDisconnect* disconn = new ipctest::CommandDisconnect;
        disconn->execute(context_);
        testBase_->setConnected(false);
        connectButton_->set_label("Connect");
        log(disconn);
        statusBar_->pop();
        statusIcon_->set(pixDisconnected_);
    }
}


void MainWindow::onExecute()
{
    Command* cmd = testBase_->getWorkCommand();
    if (!cmd)
        return;

    guiToParams(cmd->getParams());
    cmd->initParams();
    
    // Make a string array based on the message's field values.
    // This only needs to be done if command is going to use these values,
    // but its being done every time now.
    std::vector<std::string> flds;
    Gtk::TreeModel::iterator iter = messageTableList_->children().begin();
    for ( ; iter != messageTableList_->children().end(); ++iter)
    {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring val = row[mtlColumns_.colFieldValue_];
        std::cout << "value: " << val << std::endl;
        flds.push_back(val.raw());
    }

    context_.setFieldValues(flds);

//    std::cout << "Executed command " << cmd->toString() << std::endl;
    if (cmd->execute(context_))
    {
        flds = context_.getFieldValues();
        stringsToFields(flds);

        log(cmd);
    }
    else
        log(new CommandComment("Command failed: " + cmd->getName()));
}

void MainWindow::onCommandChanged()
{
    const Gtk::TreeModel::iterator iter = commands_->get_active();
    if (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring cmdName = row[commandColumns_.colName_];
        std::cout << "Work command is " << cmdName << std::endl;
        Command* cmd = testBase_->getWorkCommand();
        if (!cmd || (cmd->getName() != cmdName))
        {
            Message* msg = context_.getMessage();
            cmd = testBase_->createCommand(cmdName, msg);
            std::cout << "Set work command " << cmdName << std::endl;
            testBase_->setWorkCommand(cmd);
        }
        else
            std::cout << "Reusing work command " << cmdName << std::endl;

        paramsToGui(cmd->getParams());
    }
}

void MainWindow::onMessageActivated(const Gtk::TreeModel::Path& path,
                                    Gtk::TreeViewColumn* column)
{
    std::cout << "Message selected" << std::endl;
}

void MainWindow::onMessageSelection()
{
    std::cout << "Message selection changed" << std::endl;
    Glib::RefPtr<Gtk::TreeSelection> selection = messageListView_->get_selection();
    Gtk::TreeModel::Row row = *(selection->get_selected());
    Glib::ustring msgName = row[mlColumns_.colName_];
    messageName_->set_text(msgName);

    void * ptr = row[mlColumns_.colMessage_];
    Message* msg = (Message *) ptr;
    context_.setMessage(msg);
//    testBase_->setWorkMessage(msg);
    Command* cmd = testBase_->getWorkCommand();
    if (cmd)
        cmd->setMessage(msg);

    messageTableList_->clear();

    FieldsArray fields = msg->getFields();
    FieldsIterator fi;
    for (fi = fields.begin(); fi != fields.end(); ++fi)
    {
        Field* fld = *fi;
        row = *(messageTableList_->append());
        row[mtlColumns_.colFieldName_] = fld->name();
        row[mtlColumns_.colField_] = fld;
    }
}


void MainWindow::onOpen()
{
    Gtk::FileChooserDialog dlg("Please choose a test case to open",
                               Gtk::FILE_CHOOSER_ACTION_OPEN);
    dlg.set_transient_for(*this);
    dlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dlg.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

    Glib::RefPtr<Gtk::FileFilter> filter_testsuite = Gtk::FileFilter::create();
    filter_testsuite->set_name("Ipctest Testsuite files");
    filter_testsuite->add_pattern("*.its");
    Glib::RefPtr<Gtk::FileFilter> filter_any = Gtk::FileFilter::create();
    filter_any->set_name("Any files");
    filter_any->add_pattern("*");
    dlg.add_filter(filter_testsuite);
    dlg.add_filter(filter_any);

    int result = dlg.run();
    switch (result)
    {
    case Gtk::RESPONSE_OK:
        std::string filename = dlg.get_filename();
        if (!filename.empty() && filename != testBase_->getFileName())
        {
            if (testBase_->deserialize(filename))
            {
                historyList_->clear();
                CommandList& commandList = testBase_->commandList();
                CommandIterator it = commandList.begin();
                for ( ; it != commandList.end(); ++it)
                    historyList_->add(*it);

                docModified_ = false;
                set_title(filename);
            }
        }
        break;
    }

}

void MainWindow::onRun()
{
    ipctest::CommandList* commands = historyList_->getCommands();
    if (!commands->empty())
    {
        context_.setCommands(commands);
        ipctest::CommandIterator it = context_.getCommandIterator();
        for ( ; it != commands->end(); ++it)
        {
            ipctest::Command* cmd = *it;
            context_.setCommandIterator(it);
            LOG << "Execute from onRun:" << std::endl;
            cmd->execute(context_);
            it = context_.getCommandIterator();
        }
    }
}

void MainWindow::onDemote()
{
    ipctest::Command::bumpLevel(-1);
}

void MainWindow::onPromote()
{
    ipctest::Command::bumpLevel(+1);
}

void MainWindow::onMoveDown()
{

}

void MainWindow::onMoveUp()
{

}

void MainWindow::onQuit()
{
    programQuitting_ = true;

    // check to save testcase before quitting.
    if (docModified_)
    {
            Gtk::MessageDialog dlg(*this, "Document has been modified.  "
                                   "Do you you want to save changes before exiting?",
                                   false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
        
            if (dlg.run() == Gtk::RESPONSE_OK)
                onSave();
    }

    hide();
}

void MainWindow::onSave()
{
    if (testBase_->getFileName().empty())
        onSaveAs();
    if (testBase_->getFileName().empty())
        return;

    std::string filename(testBase_->getFileName());
    if (filename.find('.') == filename.npos)
    {
        filename += ".its";
    }

    // warn if file exists, before overwriting
    std::ifstream fi(filename.c_str());
    if (fi.is_open())
    {
        fi.close();
        if (!programQuitting_)
        {
            Gtk::MessageDialog dlg(*this, "File already exists.  "
                                   "Are you sure you want to overwrite this file?",
                                   false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
        
            if (dlg.run() != Gtk::RESPONSE_OK)
            {
                testBase_->setFileName("");
                return;
            }
        }
    }

    testBase_->setFileName(filename);
    testBase_->serialize();

    set_title(filename);
}

void MainWindow::onSaveAs()
{
    Gtk::FileChooserDialog dlg("Please choose a file to save test case",
                               Gtk::FILE_CHOOSER_ACTION_SAVE);
    dlg.set_transient_for(*this);
    dlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dlg.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

    Glib::RefPtr<Gtk::FileFilter> filter_testsuite = Gtk::FileFilter::create();
    filter_testsuite->set_name("Ipctest Testsuite files");
    filter_testsuite->add_pattern("*.its");
    Glib::RefPtr<Gtk::FileFilter> filter_any = Gtk::FileFilter::create();
    filter_any->set_name("Any files");
    filter_any->add_pattern("*");
    dlg.add_filter(filter_testsuite);
    dlg.add_filter(filter_any);

    int result = dlg.run();
    switch (result)
    {
    case Gtk::RESPONSE_OK:
        std::string filename = dlg.get_filename();
        if (!filename.empty() && filename != testBase_->getFileName())
        {
            testBase_->setFileName(filename);
            onSave();
        }
        break;
    }
}
