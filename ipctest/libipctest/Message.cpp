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

// Message.cpp
//

#include "Field.h"
#include "Message.h"
using namespace ipctest;


// Initialize static member variables:
int Message::ordinalSeq = 1;


Message::Message(const std::string& name, int ordinal)
    : name_(name)
    , ordinal_(ordinal==-1? ordinalSeq++:ordinal)
    , totalSize_(0)
{

}

Message::~Message()
{
    fields_.clear();
}

void Message::addField(Field* field)
{
    fields_.push_back(field);
    totalSize_ += field->size() * field->elements();
}

const FieldsArray& Message::getFields() const
{
    return fields_;
}

const std::string& Message::getName() const
{
    return name_;
}

const int Message::getOrdinal() const
{
    return ordinal_;
}

const int Message::getSize() const
{
    return totalSize_;
}
