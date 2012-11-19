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

#include <iostream>
#include <sstream>

#include "Field.h"
#include "Message.h"
#include "Params.h"
using namespace ipctest;

#include <string.h>


// Initialize static member variables:
int Message::ordinalSeq = 1;
int Message::messageSeq = 1;


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

int Message::bumpSequence(bool incr)
{
    if (incr)
        return messageSeq++;
    else
        return messageSeq;
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


int Message::packFields(const std::vector<std::string>& vals, char* buf) const
{
    int idx = 0;
    FieldsConstIterator fi = fields_.begin();
    std::vector<std::string>::const_iterator it = vals.begin();
    for ( ; (fi != fields_.end()) && (it != vals.end()); ++fi, ++it)
    {
        Field* fld = *fi;
        std::string strval = *it;
//        std::cout << " idx = " << idx << std::endl;
        fld->type().fromString(strval, &buf[idx], fld->elements());

        idx += fld->size() * fld->elements()
            +  (idx & 1);		// Align on 2 bytes
    }
  
    return idx;
}


int Message::packParams(const Params& params, char* buf) const
{
    memset(buf, 0, getSize());

    if (params.empty())
        return getSize();

    int idx = 0;
    FieldsConstIterator fi = fields_.begin();
    for ( ; fi != fields_.end(); ++fi)
    {
        Field* fld = *fi;
        std::string strval;
        if (params.get(fld->name(), strval))
            fld->type().fromString(strval, &buf[idx], fld->elements());

        idx += fld->size() * fld->elements()
            +  (idx & 1);		// Align on 2 bytes
    }

    return idx;
}


int Message::unpackFields(const char* buf, std::vector<std::string>& vals) const
{
    int idx = 0;

    vals.clear();
    FieldsConstIterator fi = fields_.begin();
    for ( ; fi != fields_.end(); ++fi)
    {
        Field* fld = *fi;
        std::string strval;
//        std::cout << " idx = " << idx << std::endl;
        fld->type().toString(&buf[idx], strval, fld->elements());

        vals.push_back(strval);
        idx += fld->size() * fld->elements()
            +  (idx & 1);		// Align on 2 bytes
    }
  
    return idx;
}

int Message::unpackParams(char* buf, Params& params) const
{
    std::vector<std::string> vals;

    if (unpackFields(buf, vals) > 0)
        return unpackParams(vals, params);

    return 0;
}

int Message::unpackParams(const std::vector<std::string>& vals, Params& params) const
{
    int idx = 0;

    if (vals.size() != fields_.size())
        std::cout << "Error, unpackParams got mismatch in fields and values" << std::endl;

    std::vector<std::string>::const_iterator vi(vals.begin());
    FieldsConstIterator fi = fields_.begin();
    for ( ; fi != fields_.end() && vi != vals.end(); ++fi, ++vi)
    {
        Field* fld = *fi;
        params.set(fld->name(), *vi);

        idx++;
    }

    return idx;
}


std::string Message::toXml(int indent, const char* buf) const
{
    std::string idstr(indent, ' ');
    if (!buf)
        return idstr + "<Message/>\n";

    std::ostringstream str;
    int idx = 0;

    str << idstr << "<Message>" << std::endl;
    FieldsConstIterator fi = fields_.begin();
    for ( ; fi != fields_.end(); ++fi)
    {
        Field* fld = *fi;
        std::string strval;
//        std::cout << " idx = " << idx << std::endl;
        fld->type().toString(&buf[idx], strval, fld->elements());

        str << idstr << "    <Field name=\"" << fld->name() << "\">" << std::endl
            << strval << std::endl << "    </Field>" << std::endl;

        idx += fld->size() * fld->elements()
            +  (idx & 1);		// Align on 2 bytes
    }
    str << idstr << "</Message>" << std::endl;

    return str.str();
}
