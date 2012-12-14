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
class Command;
class Field;
class Params;
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
// TYPE DEFINITIONS
//

// FORWARD CLASS DECLARATIONS
class HistoryList;

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
        add(colField_);
        add(colFieldValue_);
    }

    Gtk::TreeModelColumn<Glib::ustring> colFieldName_;
    Gtk::TreeModelColumn<ipctest::Field*> colField_;
    Gtk::TreeModelColumn<Glib::ustring> colFieldValue_;
};



class MainWindow : public Gtk::Window
{
public:
    MainWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
    virtual ~MainWindow();

    void log(ipctest::Command* cmd);
    void guiToParams(ipctest::Params* params);
    void paramsToGui(ipctest::Params* params);
    void setCommand(ipctest::Command* cmd);
    bool setup(const std::string& defFilename, const std::string& testFilename);

private:
    MainWindow(const MainWindow&);	// disable copy constructor
    MainWindow& operator=(const MainWindow& rSource);	// disable assignment operator

private:
    bool initDialog();
    void stringsToFields(const std::vector<std::string>& flds);

	// signal handlers
    void onCellDataFieldType(Gtk::CellRenderer* renderer,
                             const Gtk::TreeModel::iterator& iter);
    void onConnect();
    void onExecute();
    void onCommandChanged();
    void onMessageActivated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);
    void onMessageSelection();
    void onOpen();
    void onRun();
    void onDemote();
    void onPromote();
    void onMoveDown();
    void onMoveUp();
    void onQuit();
    void onSave();
    void onSaveAs();

    Glib::RefPtr<Gtk::Builder> builder_;
    CommandColumns commandColumns_;
    MessageListColumns mlColumns_;
    MessageTableListColumns mtlColumns_;

    Gtk::Grid* inputGrid_;
    Gtk::ComboBox* commands_;
    Glib::RefPtr<Gtk::ListStore> commandList_;
    Gtk::Entry* messageName_;
    Gtk::TreeView* messageListView_;
    Glib::RefPtr<Gtk::ListStore> messageList_;
    Gtk::TreeView* messageTableView_;
    Glib::RefPtr<Gtk::ListStore> messageTableList_;

    Gtk::CellRendererText fieldTypeRenderer_;
    Gtk::TreeView::Column fieldTypeColumn_;

    Gtk::Statusbar*  statusBar_;
//    Glib::RefPtr<Gtk::StatusIcon> statusIcon_;
//    Gtk::StatusIcon* statusIcon_;
    Gtk::Image* statusIcon_;
    Gtk::Button* connectButton_;
    Glib::RefPtr<Gdk::Pixbuf> pixConnected_;
    Glib::RefPtr<Gdk::Pixbuf> pixDisconnected_;

    HistoryList* historyList_;
    ipctest::RunContext& context_;
    ipctest::TestBase*  testBase_;

    std::stack<Gtk::Widget*> inputWidgets_;

    bool docModified_;
    bool programQuitting_;
};

}  // namespace uigtk
}  // namespace ipctest

#endif
