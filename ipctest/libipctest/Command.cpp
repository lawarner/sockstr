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

// Command.cpp
//
#include <iostream>
#include "Command.h"
#include "Message.h"
#include "RunContext.h"
using namespace ipctest;

#include <string.h>

// static member initialization
Message* Command::emptyMessage_ = new Message("EmptyMessage");
std::map<std::string, Command*> Command::sCommands_;



// static
Command* Command::createCommand(const std::string& cmdName, 
                                Params* params, Message* msg)
{	// lookup the globally registered command
    std::map<std::string, Command*>::iterator it = sCommands_.find(cmdName);
    if (it == sCommands_.end())
        return 0;

    //TODO: use "_commandName" param in subclass creation
    if (params)
        params->set("_commandName", cmdName);

    // use command found to dispatch the correct subclasses 
    // virtual createCommand method.
    Command* cmd = it->second;
    Command* newCmd = cmd->createCommand(params, msg);
    return newCmd;
}


void Command::registerCommand(Command* cmd)
{
    sCommands_[cmd->getName()] = cmd;
    std::cout << "register command " << cmd->getName() << std::endl;
}
