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
class FieldTypeInt32;
class FieldTypeInt64;
class FieldTypeChar;
class FieldTypeStruct;
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
    enum FieldIdent
    {
        Undefined,
        Int16,
        Int32,
        Int = Int32,
        Int64,
        Char,
        Struct
    };

	FieldType(FieldIdent ft, int sz) : ftype_(ft), size_(sz) { }

    static FieldType& stringToType(const std::string& ftStr);
    static std::string typeToString(const FieldType& fType);
    
    // Convert between string and internal representation.
    // The internal format depends on the type (i.e. FieldIdent) of the data.
    virtual bool   toString(const void* raw,
                            std::string& strval, int elements = 1) const;
    virtual bool fromString(const std::string& strval,
                            void* raw, int elements = 1) const;

    FieldIdent ident() const { return ftype_; }
    int size() const { return size_; }
    std::string toString() const { return typeToString(*this); }

protected:
    FieldIdent ftype_;
    int size_;

private:
    static FieldTypeInt gFieldTypeInt;
    static FieldTypeInt16 gFieldTypeInt16;
    static FieldTypeInt32 gFieldTypeInt32;
    static FieldTypeInt64 gFieldTypeInt64;
    static FieldTypeChar gFieldTypeChar;
    static FieldTypeStruct gFieldTypeStruct;
    static FieldTypeUndefined gFieldTypeUndefined;
};

class FieldTypeInt : public FieldType
{
public:
    FieldTypeInt() : FieldType(Int, 4) { }
    virtual bool   toString(const void* raw,
                            std::string& strval, int elements = 1) const;
    virtual bool fromString(const std::string& strval,
                            void* raw, int elements = 1) const;
};

class FieldTypeInt16 : public FieldType
{
public:
FieldTypeInt16() : FieldType(Int16, 2) { }
    virtual bool   toString(const void* raw,
                            std::string& strval, int elements = 1) const;
    virtual bool fromString(const std::string& strval,
                            void* raw, int elements = 1) const;
};

class FieldTypeInt32 : public FieldType
{
public:
FieldTypeInt32() : FieldType(Int32, 4) { }
};

class FieldTypeInt64 : public FieldType
{
public:
FieldTypeInt64() : FieldType(Int64, 8) { }
    virtual bool   toString(const void* raw,
                            std::string& strval, int elements = 1) const;
    virtual bool fromString(const std::string& strval,
                            void* raw, int elements = 1) const;
};

class FieldTypeChar : public FieldType
{
public:
FieldTypeChar() : FieldType(Char, 1) { }
};

class FieldTypeStruct : public FieldType
{
public:
FieldTypeStruct() : FieldType(Struct, 0) { }
    virtual bool   toString(const void* raw,
                            std::string& strval, int elements = 1) const;
    virtual bool fromString(const std::string& strval,
                            void* raw, int elements = 1) const;
};

class FieldTypeUndefined : public FieldType
{
public:
FieldTypeUndefined() : FieldType(Undefined, 0) { }
};


}  // namespace ipctest

#endif
