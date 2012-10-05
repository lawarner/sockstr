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
#include <expat.h>
#include "BuiltinCommands.h"
#include "Parser.h"
#include "TestBase.h"
using namespace ipctest;
using namespace std;


static int sLevel;


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
    else if (cmdName == "Noop")
        cmd = new CommandNoop;
    else if (cmdName == "Receive")
        cmd = new CommandReceive(msg);
    else if (cmdName == "Send")
        cmd = new CommandSend(msg, msgData);

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


static void _deserialStartTag(void* data, const char* el, const char** attr)
{
    std::string str(sLevel * 4, ' ');
    std::cout << str << "<" << el << ">" << std::endl;
    sLevel++;
}

static void _deserialEndTag(void* data, const char* el)
{
    sLevel--;
}

static void _deserialCharData(void *data, const char *txt, int txtlen)
{
    if (txtlen < 1) return;

    std::string str(txt, txtlen);
    std::cout << "-+- data: " << str << std::endl;
}

static void _deserialPI(void *data, const char *target, const char *pidata)
{
    std::cout << " xml PI target=" << target << ", pidata=" << pidata << std::endl;
}


bool TestBase::deserialize(const std::string& fileName)
{
    std::ifstream ifile(fileName.c_str());
    if (!ifile.is_open())
        return false;

    string itsFile((istreambuf_iterator<char>(ifile)),
                      istreambuf_iterator<char>());
    ifile.close();

    std::string magic("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<ipctest>");
    if (itsFile.compare(0, magic.size(), magic))
    {
        std::cerr << "File " << fileName << " is not an ipctest file." << std::endl;
        return false;
    }

    XML_Parser p = XML_ParserCreate(NULL);
    if (!p) return false;

    sLevel = 0;		// level of nesting while parsing XML

    XML_UseParserAsHandlerArg(p);
    XML_SetElementHandler(p, _deserialStartTag, _deserialEndTag);
    XML_SetCharacterDataHandler(p, _deserialCharData);
    XML_SetProcessingInstructionHandler(p, _deserialPI);

    XML_Parse(p, itsFile.c_str(), itsFile.length(), 1);

    XML_ParserFree(p);

    fileName_ = fileName;
    return true;
}


bool TestBase::serialize()
{
    std::ofstream fo(fileName_.c_str());
    if (!fo.is_open())
        return false;

    fo << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl
       << "<ipctest>\n  <!-- Ipctest Testsuite file version 1.0 -->" << std::endl;

    CommandIterator it = commandList_.begin();
    for ( ; it != commandList_.end(); ++it)
        fo << (*it)->toXml(4);

    fo << "</ipctest>" << std::endl;
    return true;
}
