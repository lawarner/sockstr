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

// BuiltinCommands.cpp
//
#include <sockstr/Socket.h>

#include "BuiltinCommands.h"
#include "RunContext.h"
using namespace ipctest;


// Connect
CommandConnect::CommandConnect(const std::string& url)
    : Command("Connect")
    , params_(new Params)
    , url_(url)
{

}


bool CommandConnect::execute(RunContext& context)
{
    sockstr::Socket* sock = new sockstr::Socket(url_.c_str(), sockstr::Socket::modeReadWrite);
    if (sock->queryStatus() != sockstr::SC_OK)
        return false;

    data_ = sock;
    context.setSocket(sock);
    return true;
}

sockstr::Socket* CommandConnect::getSocket() const
{
    return (sockstr::Socket*) data_;
}


//  Disconnect
CommandDisconnect::CommandDisconnect()
    : Command("Disconnect")
{

}

bool CommandDisconnect::execute(RunContext& context)
{
    sockstr::Socket* sock = context.getSocket();
    if (!sock)
        return false;

    sock->close();

    return true;
}


//  Function
CommandFunction::CommandFunction(const std::string& funcName, void* data, int delay)
    : Command("Function", "", data, delay)
    , functionName_(funcName)
{

}

void CommandFunction::addCommand(Command* cmd)
{
    commands_.push_back(cmd);
}

bool CommandFunction::execute(RunContext& context)
{
    CommandIterator it = commands_.begin();
    for ( ; it != commands_.end(); ++it)
        if (!(*it)->execute(context))
            return false;       // stop on first error

    return true;
}


CommandReceive::CommandReceive(const std::string& msgName)
    : Command("Receive", msgName, 0)
{

}


bool CommandReceive::execute(RunContext& context)
{
    return false;
}


CommandSend::CommandSend(const std::string& msgName, void* msgData)
    : Command("Send", msgName, msgData)
{

}

bool CommandSend::execute(RunContext& context)
{
    return false;
}
