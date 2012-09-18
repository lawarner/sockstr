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

#include <iostream>
#include <sstream>

#include "MainWindow.h"
#include "Field.h"
#include "HistoryList.h"
#include "BuiltinCommands.h"
#include "Parser.h"
#include "RunContext.h"
#include "TestBase.h"


using namespace ipctest::uigtk;


MainWindow::MainWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::Window(cobject)
    , builder_(builder)
    , context_(*new ipctest::RunContext)
    , testBase_(new ipctest::TestBase)
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

    log(new CommandComment("initDialog complete."));

    return true;
}


void MainWindow::log(ipctest::Command* cmd)
{
    std::cout << "LOG: " << cmd->toString() << std::endl;
    historyList_->add(cmd);
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
        std::cout << "+guiToParam=" << it->first << std::endl;
        if (inEntry)
        {
            std::string str = inEntry->get_text();
            std::cout << "     ++ got widget value=" << str << std::endl;
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
        if (wid)
            delete wid;
        inputWidgets_.pop();
    }

    if (!params)
        return;

    Gtk::Label* endLabel;
    builder_->get_widget("endlabel", endLabel);
    const ParamMap& pm = params->getAllParams();
    ParamMap::const_iterator it;
    for (it = pm.begin(); it != pm.end(); ++it)
    {
        std::cout << "+paramsToGui param=" << it->first << std::endl;
        Gtk::Label* inLabel = new Gtk::Label(it->first + ":");
        inLabel->show();
        inputGrid_->attach_next_to(*inLabel, *endLabel, Gtk::POS_BOTTOM, 1, 1);
        inputWidgets_.push(inLabel);
        Gtk::Entry* inEntry = new Gtk::Entry;
        inEntry->show();
        inputGrid_->attach_next_to(*inEntry, *inLabel, Gtk::POS_RIGHT, 1, 1);
        inputWidgets_.push(inEntry);
        it->second->widget = inEntry;
//        params->setWidget(it->first, inEntry);

        endLabel = inLabel;
    }

}


void MainWindow::setCommand(const std::string& cmdName)
{
    Glib::RefPtr<Gtk::TreeModel> cmdModel = commands_->get_model();
    Gtk::TreeModel::iterator iter = cmdModel->children().begin();
    for ( ; iter != cmdModel->children().end(); ++iter)
    {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring cmd = row[commandColumns_.colName_];
        if (cmd.raw() == cmdName)
        {
            commands_->set_active(iter);
            break;
        }
    }
}

bool MainWindow::setup(const std::string& defFilename)
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

    return true;
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
    ipctest::CommandIterator it;

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
    cmd->execute(context_);
    std::cout << "Executing command " << cmd->toString() << std::endl;
    log(cmd);
}

void MainWindow::onCommandChanged()
{
    const Gtk::TreeModel::iterator iter = commands_->get_active();
    if (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring cmdName = row[commandColumns_.colName_];
        std::cout << "Work command is " << cmdName << std::endl;
        Command* cmd = testBase_->createCommand(cmdName);
        testBase_->setWorkCommand(cmd);

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
    context_.setValue("Message Name", msgName.raw());

    void * ptr = row[mlColumns_.colMessage_];
    Message* msg = (Message *) ptr;
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
