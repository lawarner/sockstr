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

// RunContext.cpp
//
#include <sockstr/Socket.h>
#include <algorithm>

#include "RunContext.h"
using namespace ipctest;


RunContext::RunContext()
    : commands_(0)
    , sock_(0)
    , message_(0)
{

}


CommandList* RunContext::getCommands()
{
    return commands_;
}

void RunContext::setCommands(CommandList* cmds)
{
    commands_ = cmds;
    if (cmds)
        iter_ = cmds->begin();
}


CommandIterator RunContext::getCommandIterator() const
{
    return iter_;
}

void RunContext::setCommandIterator(const CommandIterator& cmdIter)
{
    iter_ = cmdIter;
}

std::string RunContext::getFieldValue(const std::string& name)
{
    std::vector<std::string>::iterator it;

    it = std::find(fieldValues_.begin(), fieldValues_.end(), name);
    if (it == fieldValues_.end())
        return "";

    
}

std::vector<std::string>& RunContext::getFieldValues()
{
    return fieldValues_;
}

void RunContext::setFieldValues(std::vector<std::string>& vals)
{
    fieldValues_ = vals;
}


Message* RunContext::getMessage() const
{
    return message_;
}

void RunContext::setMessage(Message* msg)
{
    message_ = msg;
}

sockstr::Socket* RunContext::getSocket() const
{
    return sock_;
}

void RunContext::setSocket(sockstr::Socket* sock)
{
    sock_ = sock;
}

Params& RunContext::getParams()
{
    return params_;
}

std::string RunContext::getValue(const std::string& key)
{
    std::string str;
    params_.get(key, str);
    return str;
}

void RunContext::setValue(const std::string& key, const std::string& value)
{
    params_.set(key, value);
}
