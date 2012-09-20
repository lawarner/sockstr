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

// Field.cpp
//
#include "Field.h"
using namespace ipctest;
using namespace std;


Field::Field(const string& name, const FieldType& ft, int elements)
    : name_(name)
    , type_(ft)
    , size_(ft.size())
    , elements_(elements)
{

}

Field::~Field()
{

}

Field* Field::create(const std::string& name, const std::string& ftype, int occurs)
{
    Field* field = 0;
    FieldType& refFt = FieldType::stringToType(ftype);
    if (refFt.ident() != FieldType::Undefined)
    {
        field = new Field(name, refFt, occurs);
    }

    return field;
}


int Field::elements() const
{
    return elements_;
}

const FieldValue& Field::get() const
{
    return *value_;
}

FieldValue& Field::getRef() const
{
    return *value_;
}

void Field::set(const FieldValue fv)
{
    *value_ = fv;
}

int Field::size() const
{
    return size_;
}

