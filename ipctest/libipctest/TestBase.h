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

#ifndef _TESTBASE_H_INCLUDED_
#define _TESTBASE_H_INCLUDED_
//
//

//
// INCLUDE FILES
//
#include <vector>
#include "Command.h"
#include "Message.h"


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
class TestBase
{
public:
    TestBase();
    ~TestBase();

    Command* createCommand(const std::string& cmdName, 
                           const std::string& msgName, void* msgData = 0);
    bool isConnected() const { return isConnected_; }
    void setConnected(bool connected) { isConnected_ = connected; }
    sockstr::Socket* getSocket() { return socket_; }
    void setSocket(sockstr::Socket* sock) { socket_ = sock; }

    bool readIpcDefs(const std::string& fileName);
    const std::vector<std::string>& builtinCommandNames() const { return builtinCommands_; }
    const MessageList& messageList() const { return messageList_; }

private:
    void init();

private:
    Command* currentMain_;
    CommandList commandList_;
    MessageList messageList_;
    std::vector<std::string> builtinCommands_;
    bool isConnected_;
    sockstr::Socket* socket_;

private:
    TestBase(const TestBase&);	// disable copy constructor
    TestBase& operator=(const TestBase& rSource);	// disable assignment operator
};

}  // namespace ipctest

#endif
