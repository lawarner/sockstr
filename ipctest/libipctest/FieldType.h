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

#ifndef _FIELDTYPE_H_INCLUDED_
#define _FIELDTYPE_H_INCLUDED_
//
//

//
// INCLUDE FILES
//
#include <string>


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
class FieldTypeInt;
class FieldTypeInt16;
class FieldTypeChar;
class FieldTypeUndefined;

//
// TYPE DEFINITIONS
//

//
// CLASS DEFINITIONS
//
class FieldType
{
public:
    enum FieldEnum
    {
        Undefined,
        Int16,
        Int32,
        Int = Int32,
        Char
    };

	FieldType(FieldEnum ft, int sz) : ftype_(ft), size_(sz) { }

    static FieldType& fromString(const std::string& ftStr);

    int size() const { return size_; }

protected:
    FieldEnum ftype_;
    int size_;

private:
    static FieldTypeInt gFieldTypeInt;
    static FieldTypeInt16 gFieldTypeInt16;
    static FieldTypeChar gFieldTypeChar;
    static FieldTypeUndefined gFieldTypeUndefined;
};

class FieldTypeInt : public FieldType
{
public:
FieldTypeInt() : FieldType(Int, 4) { }
};

class FieldTypeInt16 : public FieldType
{
public:
FieldTypeInt16() : FieldType(Int16, 2) { }
};

class FieldTypeChar : public FieldType
{
public:
FieldTypeChar() : FieldType(Char, 1) { }
};

class FieldTypeUndefined : public FieldType
{
public:
FieldTypeUndefined() : FieldType(Undefined, 0) { }
};


}  // namespace ipctest

#endif
