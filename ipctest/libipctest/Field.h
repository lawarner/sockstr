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

#ifndef _FIELD_H_INCLUDED_
#define _FIELD_H_INCLUDED_
//
//

//
// INCLUDE FILES
//
#include <string>
#include "FieldType.h"


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
//class FieldValue;

//
// TYPE DEFINITIONS
//
typedef int FieldValue;

//
// CLASS DEFINITIONS
//
class Field
{
public:
    Field(const std::string& name, const FieldType& ft, int elements);
    virtual ~Field();

    static Field* create(const std::string& name, const std::string& ftype, int occurs = 1);

    int elements() const;
    const FieldValue& get() const;
    FieldValue& getRef() const;
    std::string name() const { return name_; }
    void set(const FieldValue fv);
    int size() const;
    const FieldType& type() const { return type_; }
    const FieldType* typePointer() const { return &type_; }

private:
    std::string name_;
    const FieldType& type_;
    int size_;
    int elements_;
    FieldValue* value_;

private:
    Field(const Field&);	// disable copy constructor
    Field& operator=(const Field& rSource);	// disable assignment operator
};

}  // namespace ipctest

#endif
