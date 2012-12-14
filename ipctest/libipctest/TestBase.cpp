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
#include "Condition.h"
#include "Parser.h"
#include "Serializer.h"
#include "TestBase.h"
using namespace ipctest;
using namespace std;



TestBase::TestBase()
    : currentMain_(new CommandFunction("main"))
    , workCommand_(0)
    , isConnected_(false)
    , socket_(0)
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
    builtinCommands_.push_back("Else");
    builtinCommands_.push_back("Exec");
    builtinCommands_.push_back("Function");
    builtinCommands_.push_back("If");
    builtinCommands_.push_back("Loop");
    builtinCommands_.push_back("Noop");
    builtinCommands_.push_back("Receive");
    builtinCommands_.push_back("Send");
    builtinCommands_.push_back("While");

    std::vector<std::string>::const_iterator it;
    for (it = builtinCommands_.begin(); it != builtinCommands_.end(); ++it)
    {
        std::string cmdName = *it;
        Command* cmd = createCommand(cmdName, 0, 0);
        if (cmd)
            Command::registerCommand(cmd);
    }
}


Command* TestBase::createCommand(const std::string& cmdName, 
                                 Message* msg, void* msgData)
{
    Command* cmd = 0;

    if (cmdName == "Comment")
        cmd = new CommandComment("");
    else if (cmdName == "Connect")
        cmd = new CommandConnect("");
    else if (cmdName == "Disconnect")
        cmd = new CommandDisconnect;
    else if (cmdName == "Function")
        cmd = new CommandFunction("", msgData);
    else if (cmdName == "If")
        cmd = new CommandIf(Condition::createCondition("false"));
    else if (cmdName == "Else")
        cmd = new CommandElse;
    else if (cmdName == "Exec")
        cmd = new CommandExec("", false);
    else if (cmdName == "Loop")
        cmd = new CommandLoop(1);
    else if (cmdName == "Noop")
        cmd = new CommandNoop;
    else if (cmdName == "Receive")
        cmd = new CommandReceive(msg);
    else if (cmdName == "Send")
        cmd = new CommandSend(msg, msgData);
    else if (cmdName == "While")
        cmd = new CommandWhile(Condition::createCondition("false"));

    return cmd;
}


Message* TestBase::lookupMessage(const std::string& msgName)
{
    Message* msg = 0;

    MessageList::iterator it;
    for (it = messageList_.begin(); it != messageList_.end(); ++it)
    {
        if ((*it)->getName() == msgName)
        {
            msg = *it;
//            cout << "Found message: " << msgName << " at " << msg << endl;
            break;
        }
    }
    return msg;
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


bool TestBase::deserialize(const std::string& fileName)
{
    Serializer serial(this);
    bool ret = serial.deserialize(fileName);
    if (ret)
        fileName_ = fileName;

    return ret;
}


bool TestBase::serialize()
{
    Serializer serial(this);
    return serial.serialize(fileName_);
}
