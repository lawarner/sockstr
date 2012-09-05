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

#ifndef _MAINWINDOW_H_INCLUDED_
#define _MAINWINDOW_H_INCLUDED_
//
//

//
// INCLUDE FILES
//
#include <gtkmm.h>


namespace ipctest
{

// FORWARD CLASS DECLARATIONS
class RunContext;
class TestBase;


namespace uigtk
{
//
// MACRO DEFINITIONS
//
#ifndef DllExport
#define DllExport
#endif

//
// FORWARD CLASS DECLARATIONS
//

//
// TYPE DEFINITIONS
//

//
// CLASS DEFINITIONS
//
class CommandColumns : public Gtk::TreeModel::ColumnRecord
{
public:
    CommandColumns()
    {
        add(colName_);
    }

    Gtk::TreeModelColumn<Glib::ustring> colName_;
};

class HistoryColumns : public Gtk::TreeModel::ColumnRecord
{
public:
    HistoryColumns()
    {
        add(colText_);
        add(colCommand_);
    }

    Gtk::TreeModelColumn<Glib::ustring> colText_;
    Gtk::TreeModelColumn<gpointer> colCommand_;

};


class MessageListColumns : public Gtk::TreeModel::ColumnRecord
{
public:
    MessageListColumns()
        {
            add(colOrdinal_);
            add(colName_);
            add(colMessage_);
        }

    Gtk::TreeModelColumn<int> colOrdinal_;
    Gtk::TreeModelColumn<Glib::ustring> colName_;
    Gtk::TreeModelColumn<gpointer> colMessage_;
};

class MessageTableListColumns : public Gtk::TreeModel::ColumnRecord
{
public:
    MessageTableListColumns()
    {
        add(colFieldName_);
        add(colFieldType_);
        add(colFieldValue_);
    }

    Gtk::TreeModelColumn<Glib::ustring> colFieldName_;
    Gtk::TreeModelColumn<gpointer> colFieldType_;
    Gtk::TreeModelColumn<Glib::ustring> colFieldValue_;
};




class MainWindow : public Gtk::Window
{
public:
    MainWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
    virtual ~MainWindow();

    bool setup(const std::string& defFilename);

private:
    MainWindow(const MainWindow&);	// disable copy constructor
    MainWindow& operator=(const MainWindow& rSource);	// disable assignment operator

private:
    bool initDialog();

    void onConnect();	// signal handlers
    void onExecute();
    void onMessageActivated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);
    void onMessageSelection();

    Glib::RefPtr<Gtk::Builder> builder_;
    CommandColumns commandColumns_;
    HistoryColumns histColumns_;
    MessageListColumns mlColumns_;
    MessageTableListColumns mtlColumns_;

    Gtk::ComboBox* commands_;
    Glib::RefPtr<Gtk::ListStore> commandList_;
    Gtk::Entry* messageName_;
    Gtk::TreeView* messageListView_;
    Glib::RefPtr<Gtk::ListStore> messageList_;
    Gtk::TreeView* messageTableView_;
    Glib::RefPtr<Gtk::ListStore> messageTableList_;
    Gtk::TreeView* historyView_;
    Glib::RefPtr<Gtk::ListStore> historyList_;

    ipctest::RunContext& context_;
    ipctest::TestBase*  testBase_;
};

}  // namespace uigtk
}  // namespace ipctest

#endif
