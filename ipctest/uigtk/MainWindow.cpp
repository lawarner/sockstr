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
#include "Parser.h"
#include "TestBase.h"


using namespace ipctest::uigtk;


MainWindow::MainWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::Window(cobject)
    , builder_(builder)
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

    historyList_ = Glib::RefPtr<Gtk::ListStore>::
        cast_dynamic(builder_->get_object("history_list"));

    row = *(historyList_->append());
    row[histColumns_.colText_] = "Comment Command";
    row[histColumns_.colCommand_] = 0;

    messageList_ = Glib::RefPtr<Gtk::ListStore>::
        cast_dynamic(builder_->get_object("message_list"));
    messageTableList_ = Glib::RefPtr<Gtk::ListStore>::
        cast_dynamic(builder_->get_object("messagetable_list"));

    builder_->get_widget("commands", commands_);
    commands_->pack_start(commandColumns_.colName_);

    builder_->get_widget("message", messageName_);

    builder_->get_widget("history_view", historyView_);
    historyView_->append_column("Command", histColumns_.colText_);

    builder_->get_widget("messagelist_view", messageListView_);
    messageListView_->append_column("Ordinal", mlColumns_.colOrdinal_);
    messageListView_->append_column("Message", mlColumns_.colName_);

    builder_->get_widget("messagetable_view", messageTableView_);
    messageTableView_->append_column("Field Name", mtlColumns_.colFieldName_);
    messageTableView_->append_column_editable("Value", mtlColumns_.colFieldValue_);


    // Connect signal handlers
    Gtk::Button* button;
    builder_->get_widget("connect_button", button);
    button->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onConnect));
    builder_->get_widget("execute_button", button);
    button->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onExecute));
    messageListView_->signal_row_activated()
        .connect(sigc::mem_fun(*this, &MainWindow::onMessageActivated));
    Glib::RefPtr<Gtk::TreeSelection> selection = messageListView_->get_selection();
    selection->signal_changed().connect(sigc::mem_fun(*this, &MainWindow::onMessageSelection));

    return true;
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
        int port;
        std::istringstream strport(connPort->get_text());
        strport >> port;
        std::cout << "Connect to url: " << url << ", port " << port << "." << std::endl;

        //TODO: if url == ":" only, connect a server socket
        ipctest::CommandConnect conn(url, port);
        conn.execute(it);
        testBase_->setSocket(conn.getSocket());
    }
    else
    {
        ipctest::CommandDisconnect disconn(testBase_->getSocket());
        disconn.execute(it);
    }
}

void MainWindow::onExecute()
{
    std::cout << "Execute." << std::endl;
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
    messageName_->set_text(row[mlColumns_.colName_]);

    void * ptr = row[mlColumns_.colMessage_];
    Message* msg = (Message *) ptr;
    messageTableList_->clear();
//    row = *(messageTableList_->append());
//    row[mtlColumns_.colFieldName_] = "Fields...";

    FieldsArray fields = msg->getFields();
    FieldsIterator fi;
    for (fi = fields.begin(); fi != fields.end(); ++fi)
    {
        Field* fld = *fi;
        row = *(messageTableList_->append());
        row[mtlColumns_.colFieldName_] = fld->name();
    }
}
