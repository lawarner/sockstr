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

// TestBase.cpp
//

#include <fstream>
#include <iostream>
#include "BuiltinCommands.h"
#include "Parser.h"
#include "TestBase.h"
using namespace ipctest;
using namespace std;


TestBase::TestBase()
    : currentMain_(new CommandFunction("main"))
    , isConnected_(false)
{
    init();
}

TestBase::~TestBase()
{

}


void TestBase::init()
{
    builtinCommands_.push_back("Comment");
    builtinCommands_.push_back("Connect");
    builtinCommands_.push_back("Disconnect");
    builtinCommands_.push_back("Function");
    builtinCommands_.push_back("Noop");
    builtinCommands_.push_back("Receive");
    builtinCommands_.push_back("Send");
}


Command* TestBase::createCommand(const std::string& cmdName, 
                                 const std::string& msgName, void* msgData)
{
    Command* cmd = 0;

    if (cmdName == "Comment")
        cmd = new CommandComment(msgName);
    else if (cmdName == "Connect")
        cmd = new CommandConnect(msgName);
    else if (cmdName == "Disconnect")
        cmd = new CommandDisconnect;
    else if (cmdName == "Function")
        cmd = new CommandFunction(msgName, msgData);
    else if (cmdName == "Noop")
        cmd = new CommandNoop;
    else if (cmdName == "Receive")
        cmd = new CommandReceive(msgName);
    else if (cmdName == "Send")
        cmd = new CommandSend(msgName, msgData);

    return cmd;
}


bool TestBase::readIpcDefs(const std::string& fileName)
{
    ifstream ifile(fileName.c_str());
    if (!ifile.is_open())
    {
        cerr << "Could not open IPC definitions file " << fileName << endl;
        return false;
    }

    string strIpcDefs((istreambuf_iterator<char>(ifile)),
                      istreambuf_iterator<char>());
    ifile.close();

    Parser parse;
    if (!parse.stringToMessageList(strIpcDefs, messageList_))
    {
        std::cerr << "Error parsing message list" << std::endl;
        return false;
    }

    return true;
}
