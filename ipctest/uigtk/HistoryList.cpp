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

#include "Command.h"
#include "HistoryList.h"

using namespace ipctest::uigtk;


HistoryList::HistoryList(Glib::RefPtr<Gtk::ListStore> histList, Gtk::TreeView* histView)
    : historyList_(histList)
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

    row = *(historyList_->append());
    row[histColumns_.colText_] = cmd->toString();
    row[histColumns_.colCommand_] = cmd;
}


void HistoryList::clear()
{
    commands_.clear();
    historyList_->clear();
}


void HistoryList::setup()
{
    historyView_->append_column("Command", histColumns_.colText_);

    // Connect signal handlers

}
