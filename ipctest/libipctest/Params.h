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

#ifndef _PARAMS_H_INCLUDED_
#define _PARAMS_H_INCLUDED_
//
//

//
// INCLUDE FILES
//
#include <map>
#include <string>
#include <vector>

//
// FORWARD CLASS DECLARATIONS
//
namespace sockstr
{
    class Socket;
}


namespace ipctest
{

//
// MACRO DEFINITIONS
//
#ifndef DllExport
#define DllExport
#endif

//
// TYPE DEFINITIONS
//

//
// CLASS DEFINITIONS
//
class Params
{
public:
    Params();
    ~Params();

    bool get(const std::string& name, int& value);
    bool get(const std::string& name, std::string& value);
    bool set(const std::string& name, int value);
    bool set(const std::string& name, const std::string& value);

private:
    std::map<std::string, std::string> env_;

private:
    Params(const Params&);	// disable copy constructor
    Params& operator=(const Params& rSource);	// disable assignment operator
};


}  // namespace ipctest

#endif
