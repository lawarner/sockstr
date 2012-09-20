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

// FieldType.cpp
//

#include "FieldType.h"
using namespace ipctest;


FieldTypeInt FieldType::gFieldTypeInt;
FieldTypeInt16 FieldType::gFieldTypeInt16;
FieldTypeInt32 FieldType::gFieldTypeInt32;
FieldTypeInt64 FieldType::gFieldTypeInt64;
FieldTypeChar  FieldType::gFieldTypeChar;
FieldTypeStruct FieldType::gFieldTypeStruct;
FieldTypeUndefined FieldType::gFieldTypeUndefined;


FieldType& FieldType::stringToType(const std::string& ftStr)
{
    if (ftStr == "int")
        return gFieldTypeInt;
    else if (ftStr == "int16")
        return gFieldTypeInt16;
    else if (ftStr == "int32")
        return gFieldTypeInt32;
    else if (ftStr == "int64")
        return gFieldTypeInt64;
    else if (ftStr == "char")
        return gFieldTypeChar;
    else if (ftStr == "struct")
        return gFieldTypeStruct;

    //TODO: check for extensions
    return gFieldTypeUndefined;
}


std::string FieldType::typeToString(const FieldType& fType)
{
    const char* ret = "?err";

#define STRNAME(NAME) case NAME: ret = #NAME; break

    switch (fType.ident())
    {
        STRNAME(Undefined);
        STRNAME(Int16);
        STRNAME(Int32);
        STRNAME(Int64);
        STRNAME(Char);
        STRNAME(Struct);
#if 0
    case Undefined:
        ret = "Undefined";
        break;
    case Int16:
        ret = "Int16";
        break;
    case Int32:    // note: Int == Int32
        ret = "Undefined";
        break;
    case Int64:
        ret = "Undefined";
        break;
    case Char:
        ret = "Undefined";
        break;
#endif
    default:
        break;
    }

    return ret;
}

bool FieldType::toString(void* raw, std::string& str)
{
    return false;
}

bool FieldType::fromString(std::string& str, void* raw)
{
    return false;
}

