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

// HistoryList.cpp
//

#include <iostream>

#include "BuiltinCommands.h"
#include "HistoryList.h"
#include "MainWindow.h"


using namespace ipctest::uigtk;


HistoryList::HistoryList(MainWindow* mainWind,
                         Glib::RefPtr<Gtk::ListStore> histList, Gtk::TreeView* histView)
    : mainWindow_(mainWind)
    , historyList_(histList)
    , historyView_(histView)
{
    setup();
}

HistoryList::~HistoryList()
{

}

void HistoryList::add(const std::string& str)
{
    add(new CommandComment(str));
}

void HistoryList::add(ipctest::Command* cmd)
{
    commands_.push_back(cmd);

    Gtk::TreeModel::Row row;

    std::string cmdstr(cmd->getLevel() * 2, ' ');
    cmdstr += cmd->toString();

    row = *(historyList_->append());
    row[histColumns_.colText_] = cmdstr;
    row[histColumns_.colCommand_] = cmd;

    Gtk::TreeModel::iterator iter = historyList_->children().end();
    --iter;
    historyView_->scroll_to_row(Gtk::TreePath(iter));
}


void HistoryList::clear()
{
    commands_.clear();
    historyList_->clear();
}


ipctest::CommandList* HistoryList::getCommands()
{
    return &commands_;
}


void HistoryList::setup()
{
    historyView_->append_column("Command", histColumns_.colText_);

    // Connect signal handlers
    historyView_->signal_row_activated()
        .connect(sigc::mem_fun(*this, &HistoryList::onHistoryActivated));
    Glib::RefPtr<Gtk::TreeSelection> selection = historyView_->get_selection();
    selection->signal_changed().connect(sigc::mem_fun(*this, &HistoryList::onHistorySelection));
}


// Signal handlers :
void HistoryList::onHistoryActivated(const Gtk::TreeModel::Path& path,
                                    Gtk::TreeViewColumn* column)
{
    std::cout << "History selected" << std::endl;
}

void HistoryList::onHistorySelection()
{
    std::cout << "History selection changed" << std::endl;
    Glib::RefPtr<Gtk::TreeSelection> selection = historyView_->get_selection();
    if (selection->count_selected_rows() > 0)
    {
        Gtk::TreeModel::Row row = *(selection->get_selected());
        ipctest::Command* cmd = row[histColumns_.colCommand_];
        if (cmd)
        {
            mainWindow_->setCommand(cmd);
            mainWindow_->paramsToGui(cmd->getParams());
        }
    }
}
