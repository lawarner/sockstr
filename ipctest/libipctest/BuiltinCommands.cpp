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


// Comment
CommandComment::CommandComment(const std::string& comment)
    : Command("Comment", new Params)
{
    params_->set("Comment", comment);
}

bool CommandComment::execute(RunContext& context)
{
    std::cout << "exec: " << toString() << std::endl;
    return true;
}

std::string CommandComment::toString() 
{
    std::string str;
    params_->get("Comment", str);
    return commandName_ + ": " + str; 
}


// Connect
CommandConnect::CommandConnect(const std::string& url)
    : Command("Connect", new Params)
{
    params_->set("Url", url);
}


bool CommandConnect::execute(RunContext& context)
{
    std::string url = params_->get("Url");
    sockstr::Socket* sock = new sockstr::Socket(url.c_str(), sockstr::Socket::modeReadWrite);
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

std::string CommandConnect::toString()
{
    return commandName_ + ": " + params_->get("Url");
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
    : Command("Function", new Params, data, delay)
{
    params_->set("Function Name", funcName);
}

void CommandFunction::addCommand(Command* cmd)
{
    commands_.push_back(cmd);
}

bool CommandFunction::execute(RunContext& context)
{
    std::string funcName;
    params_->get("Function Name", funcName);
    std::cout << "exec function: " << funcName << std::endl;

    CommandIterator it = commands_.begin();
    for ( ; it != commands_.end(); ++it)
        if (!(*it)->execute(context))
            return false;       // stop on first error

    return true;
}

std::string CommandFunction::toString() 
{
    std::string str;
    params_->get("Function Name", str);
    return commandName_ + ": " + str; 
}


CommandReceive::CommandReceive(const std::string& msgName)
    : Command("Receive", msgName, 0)
{

}


bool CommandReceive::execute(RunContext& context)
{
    return false;
}


// Send
CommandSend::CommandSend(const std::string& msgName, void* msgData)
    : Command("Send", msgName, msgData)
{

}

bool CommandSend::execute(RunContext& context)
{
    std::string msgName = context.getValue("Message Name");
    if (!msgName.empty())
        messageName_ = msgName;
    else if (messageName_.empty())
        return false;
    messageName_ = msgName;

    std::cout << "exec: " << toString() << std::endl;
    return true;
}

std::string CommandSend::toString()
{
    return commandName_ + ": " + messageName_;
}
