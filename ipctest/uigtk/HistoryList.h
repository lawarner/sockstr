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

#ifndef _HISTORYLIST_H_INCLUDED_
#define _HISTORYLIST_H_INCLUDED_
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
typedef std::vector<Command *> CommandList;


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
class MainWindow;

//
// CLASS DEFINITIONS
//
class HistoryColumns : public Gtk::TreeModel::ColumnRecord
{
public:
    HistoryColumns()
    {
        add(colText_);
        add(colCommand_);
    }

    Gtk::TreeModelColumn<Glib::ustring> colText_;
    Gtk::TreeModelColumn<ipctest::Command*> colCommand_;
};


//
class HistoryList
{
public:
    HistoryList(MainWindow* mainWind, Glib::RefPtr<Gtk::ListStore> histList,
                Gtk::TreeView* histView);
    virtual ~HistoryList();

    void add(const std::string& str);
    void add(ipctest::Command* cmd);
    void clear();
    CommandList* getCommands();

private:
    HistoryList(const HistoryList&);	// disable copy constructor
    HistoryList& operator=(const HistoryList& rSource);	// disable assignment operator

private:
    void setup();

    // Signal handlers
    void onHistoryActivated(const Gtk::TreeModel::Path& path,
                            Gtk::TreeViewColumn* column);
    void onHistorySelection();

private:
    MainWindow* mainWindow_;
    Glib::RefPtr<Gtk::ListStore> historyList_;
    Gtk::TreeView* historyView_;

    HistoryColumns histColumns_;

    // The list of commands is stored in a std::list.  For many GUI toolkits (GTK included)
    // the list will also be stored inside the widget internally.
    CommandList commands_;
};

}  // namespace uigtk
}  // namespace ipctest

#endif
