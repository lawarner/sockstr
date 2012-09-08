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

#ifndef _RUNCONTEXT_H_INCLUDED_
#define _RUNCONTEXT_H_INCLUDED_
//
//

//
// INCLUDE FILES
//
#include <map>
#include <string>
#include <vector>

#include "Command.h"


//
// FORWARD CLASS DECLARATIONS
//
namespace sockstr
{
    class Socket;
}
namespace ipctest
{
    class RunContext;
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
class RunContext
{
public:
    RunContext();

    CommandIterator getCommandIterator() const;
    void setCommandIterator(const CommandIterator& cmdIter);
    sockstr::Socket* getSocket() const;
    void setSocket(sockstr::Socket* sock);
    std::string getValue(const std::string& key);
    void setValue(const std::string& key, const std::string& value);

private:
    CommandIterator iter_;
    sockstr::Socket* sock_;
    std::map<std::string, std::string> env_;

private:
    RunContext(const RunContext&);	// disable copy constructor
    RunContext& operator=(const RunContext& rSource);	// disable assignment operator
};


}  // namespace ipctest

#endif
