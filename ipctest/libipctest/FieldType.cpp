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

#include <iostream>
#include <sstream>
#include <string.h>             // for memcpy()
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
        // note: Int == Int32
        STRNAME(Int32);
        STRNAME(Int64);
        STRNAME(Char);
        STRNAME(Struct);
    default:
        break;
    }

    return ret;
}


bool FieldType::toString(const void* raw, std::string& strval, int elements) const
{
    const char* buf = (const char*) raw;
    strval = std::string(buf, strlen(buf));
    std::cout << __PRETTY_FUNCTION__ << " value=" << strval << std::endl;
    return true;
}

bool FieldType::fromString(const std::string& strval, void* raw, int elements) const
{
    std::cout << __PRETTY_FUNCTION__ << " value=" << strval << std::endl;
    if (strval.size() < (size_t) elements)
    {
        memcpy(raw, strval.c_str(), strval.size());
        char* rest = (char*)raw + strval.size();
        memset(rest, 0, elements - strval.size());
    }
    else
        memcpy(raw, strval.c_str(), elements);

    return true;
}


bool FieldTypeInt::toString(const void* raw, std::string& strval, int elements) const
{
    //TODO: loop thru elements
    if ((long unsigned) raw & (sizeof(int)-1))
        std::cout << "ERROR: int field is not aligned." << std::endl;

    const int* buf = (const int*) raw;
    std::ostringstream ss;
    ss << *buf;
    strval = ss.str();
    std::cout << __PRETTY_FUNCTION__ << " value=" << strval << std::endl;
    return true;
}

bool FieldTypeInt::fromString(const std::string& strval, void* raw, int elements) const
{
    int* buf = (int*) raw;
    std::istringstream(strval) >> *buf;
    std::cout << __PRETTY_FUNCTION__ << " value=" << *buf << std::endl;
    return true;
}

// I know, this is screaming for templates :)

bool FieldTypeInt16::toString(const void* raw, std::string& strval, int elements) const
{
    //TODO: loop thru elements
    const short* buf = (const short*) raw;
    std::ostringstream ss;
    ss << *buf;
    strval = ss.str();
    std::cout << __PRETTY_FUNCTION__ << " value=" << strval << std::endl;
    return true;
}

bool FieldTypeInt16::fromString(const std::string& strval, void* raw, int elements) const
{
    short* buf = (short*) raw;
    std::istringstream(strval) >> *buf;
    std::cout << __PRETTY_FUNCTION__ << " value=" << *buf << std::endl;
    return true;
}


bool FieldTypeInt64::toString(const void* raw, std::string& strval, int elements) const
{
    //TODO: loop thru elements
    const long long* buf = (const long long*) raw;
    std::ostringstream ss;
    ss << *buf;
    strval = ss.str();
    std::cout << __PRETTY_FUNCTION__ << " value=" << strval << std::endl;
    return true;
}

bool FieldTypeInt64::fromString(const std::string& strval, void* raw, int elements) const
{
    long long* buf = (long long*) raw;
    std::istringstream(strval) >> *buf;
    std::cout << __PRETTY_FUNCTION__ << " value=" << *buf << std::endl;
    return true;
}


bool FieldTypeStruct::toString(const void* raw, std::string& strval, int elements) const
{
    std::cout << __PRETTY_FUNCTION__ << " value=" << strval << std::endl;
    return false;
}

bool FieldTypeStruct::fromString(const std::string& strval, void* raw, int elements) const
{
    std::cout << __PRETTY_FUNCTION__ << " value=" << strval << std::endl;
    return false;
}

