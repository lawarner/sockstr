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

#ifndef _MESSAGE_H_INCLUDED_
#define _MESSAGE_H_INCLUDED_
//
//

//
// INCLUDE FILES
//
#include <string>
#include <vector>


namespace ipctest
{

//
// MACRO DEFINITIONS
//
#ifndef DllExport
#define DllExport
#endif

//
// FORWARD CLASS DECLARATIONS
//
class Field;
class Message;


typedef std::vector<Field *>  FieldsArray;
typedef FieldsArray::iterator FieldsIterator;
typedef std::vector<Message *> MessageList;
//
// CLASS DEFINITIONS
//
class Message
{
public:
    Message(const std::string& name, int ordinal = -1);
    virtual ~Message();

    void addField(Field* field);
    const FieldsArray& getFields() const;
    const std::string& getName() const;
    const int getOrdinal() const;
    const int getSize() const;

private:
    std::string name_;
    int ordinal_;
    int totalSize_;

    FieldsArray fields_;

    static int ordinalSeq;
};

}  // namespace sockstr

#endif
