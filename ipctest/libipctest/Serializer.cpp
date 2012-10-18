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

// Serializer.cpp
//

#include <fstream>
#include <iostream>
#include <sstream>
#include <expat.h>
#include "Command.h"
#include "Params.h"
#include "Parser.h"
#include "Serializer.h"
#include "TestBase.h"
using namespace ipctest;
using namespace std;



Serializer::Serializer(TestBase* testBase)
    : deserialData_(*new std::ostringstream)
    , testBase_(testBase)
    , parseLevel_(0)
    , section_(UnknownSection)
{

}


Serializer::~Serializer()
{
    delete &deserialData_;
}



static void _deserialStartTag(void* data, const char* el, const char** attr)
{
    Serializer* that = (Serializer *) data;
    that->deserialStartTag(el, attr);
}

void Serializer::deserialStartTag(const char* el, const char** attr)
{
    std::string str(parseLevel_ * 4, ' ');
    std::string startTag(el);
    std::cout << str << "<" << startTag << ">" << std::endl;

    deserialData_.seekp(0);

    bool validStart = true;
    switch (parseLevel_)
    {
    case 0:
        if (startTag == "ipctest")
            section_ = HeaderSection;
        else
            validStart = false;
        break;
    case 1:
        if (startTag == "messages")
            section_ = MessageSection;
        else if (startTag == "commands")
            section_ = CommandSection;
        else
        {
            section_ = UnknownSection;
            validStart = false;
        }
        break;
    case 2:
        if (section_ == CommandSection)
        {
            commandParams_.clear();
            if  (attr && *attr)
            {
                commandParams_.loadFromNameValues(attr);
                if (commandParams_.get("_message", messageName_))
                {
                    cout << " CommandSection message=" << messageName_ << endl;
                    commandMessage_ = testBase_->lookupMessage(messageName_);
                }
                else
                    commandMessage_ = 0;
            }
        }
        break;
    case 3:
        if (section_ == CommandSection && startTag == "Message")
        {
            messageFields_.clear();
        }
        break;
    case 4:
        if (section_ == CommandSection && startTag == "Field")
        {
            fieldParams_.clear();
            fieldParams_.loadFromNameValues(attr);
        }
        break;
    default:
        break;
    }

    if (!validStart)
    {
        std::cout << "Error parsing, invalid tag " << startTag
                  << " at level " << parseLevel_ << std::endl;
//        XML_StopParser((XML_Parser) data, false);
    }

    parseLevel_++;
}

static void _deserialEndTag(void* data, const char* el)
{
    Serializer* that = (Serializer *) data;
    that->deserialEndTag(el);
}

void Serializer::deserialEndTag(const char* el)
{
    parseLevel_--;
    std::string strindent(parseLevel_ * 4, ' ');

    std::string cdata;
    if (deserialData_.tellp() > 0)
    {
        std::vector<std::string> vstr = Parser::splitString(deserialData_.str());
        //TODO: check if vstr.length() == 1
        cdata = Parser::trimSpace(vstr[1]);
    }

    if (parseLevel_ == 2 && section_ == CommandSection)
    {
        commandParams_.set("_cdata", cdata);
        std::string cmdName = el;
        cout << strindent << "createCommand("
             << cmdName << ", " << messageName_ << ")" << endl;

        Message* msg = testBase_->lookupMessage(messageName_);
//        if (msg) cout << " Found message " << messageName_ << endl;
        
        Params* cmdParams = new Params(commandParams_);
        Command* cmd = Command::createCommand(cmdName, cmdParams, msg);
//        Command* cmd = testBase_->createCommand(cmdName, msg, 0);
        // the messageFields may not be complete or not in order, so iterate 
        // thru all fields in the Message.
        if (commandMessage_)
        {
            if (!cmd->getData())
                cmd->setData(new char[commandMessage_->getSize()]);
            commandMessage_->packParams(messageFields_, static_cast<char *>(cmd->getData()));
        }

        CommandList& commandList = testBase_->commandList();
        commandList.push_back(cmd);
    }
//    else if (parseLevel_ == 3 && section_ == CommandSection)
//    {
//    }
    else if (parseLevel_ == 4 && section_ == CommandSection)
    {
        // build field values
        std::string fldName(fieldParams_.get("_name"));
        std::string fldVal(cdata);
//        std::string fldVal(fieldParams_.get("_cdata"));
        messageFields_.set(fldName, fldVal);
        cout << "messageFields set " << fldName << " to " << fldVal << endl;
    }
    else
        std::cout << strindent << "</"
                  << el << "> cdata=" << cdata << std::endl;
}


static void _deserialCharData(void *data, const char *txt, int txtlen)
{
    Serializer* that = (Serializer *) data;
    that->deserialCharData(txt, txtlen);
}

void Serializer::deserialCharData(const char *txt, int txtlen)
{
    if (txtlen < 1) return;

    std::string str(txt, txtlen);
//    std::cout << deserialData_.tellp() << " -+- data: " << str << " :atad +-+" << std::endl;
    deserialData_ << str;
}

static void _deserialPI(void *data, const char *target, const char *pidata)
{
    Serializer* that = (Serializer *) data;
    that->deserialPI(target, pidata);
}

void Serializer::deserialPI(const char *target, const char *pidata)
{
    std::cout << " xml PI target=" << target << ", pidata=" << pidata << std::endl;
}


bool Serializer::deserialize(const std::string& fileName)
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

    parseLevel_ = 0;		// level of nesting while parsing XML

//    XML_UseParserAsHandlerArg(p);
    XML_SetUserData(p, this);
    XML_SetElementHandler(p, _deserialStartTag, _deserialEndTag);
    XML_SetCharacterDataHandler(p, _deserialCharData);
    XML_SetProcessingInstructionHandler(p, _deserialPI);

    CommandList& commandList = testBase_->commandList();
    commandList.clear();
    XML_Parse(p, itsFile.c_str(), itsFile.length(), 1);

    XML_ParserFree(p);

    return true;
}


bool Serializer::serialize(const std::string& fileName)
{
    std::ofstream fo(fileName.c_str());
    if (!fo.is_open())
        return false;

    fo << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl
       << "<ipctest>\n    <!-- Ipctest Testsuite file version 1.0 -->" << std::endl;

    // Optional, dump messages definitions
    fo << "    <messages>" << std::endl;

    fo << "    <commands>" << std::endl;
    CommandList& commandList = testBase_->commandList();
    CommandIterator it = commandList.begin();
    for ( ; it != commandList.end(); ++it)
        fo << (*it)->toXml(8);
    fo << "    </commands>" << std::endl;

    fo << "</ipctest>" << std::endl;
    return true;
}
