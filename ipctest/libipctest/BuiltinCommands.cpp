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
#include <sockstr/IPC.h>
#include <sockstr/Socket.h>

#include "BuiltinCommands.h"
#include "Message.h"
#include "RunContext.h"
using namespace ipctest;

#include <string.h>


// Comment
CommandComment::CommandComment(const std::string& comment)
    : Command("Comment", "Comment", comment)
{
//    params_->set("Comment", comment);
}

bool CommandComment::execute(RunContext& context)
{
    std::cout << toString() << std::endl;
    return true;
}

std::string CommandComment::toString() 
{
    std::string str;
    params_->get("Comment", str);
    return commandName_ + ": " + str; 
}

std::string CommandComment::toXml()
{
    std::string str("<" + commandName_ + ">\n");
    str += params_->get("Comment") + "\n";
    str += "</" + commandName_ + ">\n";
    return str;
}

// Connect
CommandConnect::CommandConnect(const std::string& url)
    : Command("Connect", "Url", url)
{

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


std::string CommandConnect::toXml()
{
    std::string str("<" + commandName_ + ">\n");
    str += "</" + commandName_ + ">\n";
    return str;
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
    : Command("Function", "Function Name", funcName)
{

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


std::string CommandFunction::toXml()
{
    std::string str("<" + commandName_ + ">\n");
    str += "</" + commandName_ + ">\n";
    return str;
}


CommandReceive::CommandReceive(Message* msg)
    : Command("Receive")
{
    message_ = msg;
}

bool CommandReceive::execute(RunContext& context)
{
    Message* msg = context.getMessage();
    sockstr::Socket* sock = context.getSocket();

    if (msg && sock)
    {
        message_ = msg;
        if (message_->getName().empty())
            return false;
        std::cout << "exec: " << toString() << std::endl;

        sockstr::GenericReply reply;
        sock->setAsyncMode(true); 	// Don't block if no data available
        UINT sz = sock->read(&reply, sizeof(reply));
        if (sz == 0)
            return false;

        std::cout << "Got " << sz << " bytes, function=" << reply.wFunction_
                  << ", seq=" << reply.dwSequence_ << std::endl;
        if (sz > 12)
        {
            std::vector<std::string> fldValues;
            int szdata = msg->unpackFields(reply.filler_, fldValues);
            if (szdata)
            {
                char* databuf = new char[szdata];
                memcpy(databuf, reply.filler_, szdata);
                setData(databuf);
                context.setFieldValues(fldValues);
            }
        }
    }

    return true;
}

std::string CommandReceive::toString()
{
    return commandName_ + ": " + message_->getName();
}


std::string CommandReceive::toXml()
{
    std::string str("<" + commandName_ + "message=\"" + message_->getName() + "\">\n");
    str += "</" + commandName_ + ">\n";
    return str;
}


// Send
CommandSend::CommandSend(Message* msg, void* msgData)
    : Command("Send", msgData)
{
    message_ = msg;
}

bool CommandSend::execute(RunContext& context)
{
    Message* msg = context.getMessage();
    sockstr::Socket* sock = context.getSocket();

    if (msg && sock)
    {
        std::cout << " Yes, we rock with message and socks!" << std::endl;
        message_ = msg;
        if (message_->getName().empty())
            return false;

        std::cout << "exec: " << toString() << std::endl;
        sockstr::GenericReply basicMsg;
        // Write header and payload together
        basicMsg.wFunction_   = msg->getOrdinal();
        basicMsg.wPacketSize_ = msg->getSize();
        basicMsg.dwSequence_  = msg->bumpSequence();	// cookie
        int szdata = msg->packFields(context.getFieldValues(), basicMsg.filler_);

        if (szdata > 0)
        {
            char* databuf = new char[szdata];
            memcpy(databuf, basicMsg.filler_, szdata);
            setData(databuf);
        }

        sock->write(&basicMsg, sizeof(sockstr::IpcStruct) + basicMsg.wPacketSize_);
    }

    return true;
}


std::string CommandSend::toString()
{
    return commandName_ + ": " + message_->getName();
}

std::string CommandSend::toXml()
{
    std::string str("<" + commandName_ + "message=\"" + message_->getName() + "\">\n");
    str += "</" + commandName_ + ">\n";
    return str;
}
