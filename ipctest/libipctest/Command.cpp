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
#include <sockstr/sstypes.h>
#include <sockstr/Socket.h>

#include "Command.h"
using namespace ipctest;


// Connect
CommandConnect::CommandConnect(const std::string& url, int port)
    : url_(url)
    , port_(port)
{

}


CommandIterator& CommandConnect::execute(CommandIterator& cmds)
{
    sockstr::Socket* sock = new sockstr::Socket(url_.c_str(), port_);
    if (sock->queryStatus() == sockstr::SC_OK)
        data_ = sock;

    return cmds;
}

sockstr::Socket* CommandConnect::getSocket() const
{
    return (sockstr::Socket*) data_;
}


//  Disconnect
CommandDisconnect::CommandDisconnect(sockstr::Socket* sock)
    : Command("Disconnect", sock)
{

}

CommandIterator& CommandDisconnect::execute(CommandIterator& cmds)
{
    if (data_)
    {
        sockstr::Socket* sock = (sockstr::Socket *) data_;
        sock->close();
    }

    return cmds;
}


//  Function
CommandFunction::CommandFunction(const std::string& funcName, void* data, int delay)
    : Command("Function", data, delay)
    , functionName_(funcName)
{

}

void CommandFunction::addCommand(Command* cmd)
{
    commands_.push_back(cmd);
}

CommandIterator& CommandFunction::execute(CommandIterator& cmds)
{
    CommandIterator it = commands_.begin();
    for ( ; it != commands_.end(); ++it)
        (*it)->execute(cmds);

    return cmds;
}
