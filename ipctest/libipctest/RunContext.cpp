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

#include "Field.h"
#include "Log.h"
#include "Message.h"
#include "RunContext.h"
using namespace ipctest;
using namespace std;


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


CommandIterator RunContext::getCommandIterator(int level /*= -1*/) const
{
    if (level == -1 || iter_ == commands_->end())
        return iter_;

    CommandIterator it = iter_ + 1;
    CommandIterator ij = iter_;
    for ( ; it != commands_->end(); ++it)
    {
        Command* cmd = *it;
        if (cmd->getLevel() <= level)
            break;
        ij = it;
    }

    return ij;	// the one before it
}

void RunContext::setCommandIterator(const CommandIterator& cmdIter)
{
    iter_ = cmdIter;
}

std::string RunContext::getFieldValue(const std::string& name)
{
    if (!message_)
        return "";

    //TODO: a map could be built and cached for subsequent lookups for performance

    const FieldsArray& fields = message_->getFields();
    FieldsConstIterator it;
    std::vector<std::string>::iterator itval = fieldValues_.begin();
    for (it = fields.begin(); 
         it != fields.end() && itval != fieldValues_.end(); ++it, ++itval)
    {
        Field* fld = *it;
        if (fld->name() == name)
            break;
    }
    if (it == fields.end() || itval == fieldValues_.end())
        return "";

    return *itval;
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
