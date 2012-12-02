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

// Parser.cpp
//

#include <iostream>
#include <sstream>
#include "Field.h"
#include "Message.h"
#include "Parser.h"
using namespace ipctest;
using namespace std;

//
static const string msgBegin("IPC_MESSAGE(");
static const string msgEnd("IPC_ENDMESSAGE");


Parser::Parser()
{

}

Parser::~Parser()
{

}

bool Parser::fileToMessageList(const string& fname, MessageList& ml)
{
    return false;
}

bool Parser::stringToMessageList(const string& str, MessageList& ml)
{
    string::const_iterator it = str.begin();
    PairIter in(it, str.end());
    PairIter out;
    while (getLine(in, out))
    {
        string strLine = out.get();
//        cout << "Line: " << strLine;
        if (strLine.compare(0, msgBegin.size(), msgBegin) == 0)
        {
            if (*strLine.rbegin() == ')')
            {
                string strMsg(strLine.begin() + msgBegin.size(), strLine.end() - 1);
                Message* msg = createMessage(strMsg, in);
                if (msg)
                    ml.push_back(msg);
//                cout << endl;
            }
        }
    }

    return true;
}

Message* Parser::createMessage(const std::string& msgName, PairIter& inStr)
{
//    cout << " -- Begin msg=" << msgName << endl;

    Message* msg = new Message(msgName);
    PairIter out;
    while (getLine(inStr, out))
    {
        string strLine = out.get();
//        cout << "createMessage line: " << strLine << endl;
        if (strLine.compare(0, msgEnd.size(), msgEnd) == 0)
            break;

        vector<string> tokens;
        splitTokens(strLine, tokens);
        if (tokens.size() > 1)
        {
            string fieldName = tokens[1];
            int occurs = 1;
            if (fieldName.find('[') != fieldName.npos)
            {
                vector<string> toks;
                splitTokens(fieldName, toks, "[]");
                if (toks.size() == 2)
                {
                    fieldName = toks[0];
                    istringstream(toks[1]) >> occurs;
//                    cout << fieldName << " occurs " << occurs << endl;

                }
                else
                {
                    cerr << "Field " << fieldName
                         << ": Expecting 2 toks, got " << toks.size() << endl;
                }
                
            }
            Field* field = Field::create(fieldName, tokens[0], occurs);
            msg->addField(field);
        }
    }

    return msg;
}


bool Parser::getLine(PairIter& inStr, PairIter& outLine, bool trim)
{
    string::const_iterator it = inStr.begin();
    for ( ; it != inStr.end(); ++it)
    {
        if (*it == '\n')
        {
            PairIter pi(inStr.begin(), it);
            outLine = pi;
            if (trim)
                trimSpace(outLine);
            ++it;
            inStr.set(it);
            return true;
        }
    }
    return false;
}


bool makeParseTree(const std::string& str, ParseTree& tree)
{
    return false;
}

void Parser::splitTokens(const std::string& str, vector<std::string>& strVec,
                         const char* seps)
{
    size_t curr = str.find_first_not_of(seps, 0);
    size_t next = 0;

//    cout << " Tokens: [ ";
    while (curr != str.npos)
    {
        next = str.find_first_of(seps, curr);
        if (next == str.npos)
            break;
        string strTok(str, curr, next - curr);
//        cout << strTok << ", ";
        strVec.push_back(strTok);
        curr = str.find_first_not_of(seps, next);
    }

    if (curr != str.npos)
    {
        string strTok(str, curr);
//        cout << strTok << " ";
        strVec.push_back(strTok);
    }
//    cout << "]" << endl;
}


void Parser::splitDelimitedTokens(const std::string& str, vector<std::string>& strVec,
                                  const char* seps)
{
    size_t curr = str.find_first_not_of(seps, 0);
    size_t next = 0;

    for (size_t i = 0; i < curr; i++)	// string started with a delimiter
        strVec.push_back(string(1, str[i]));

    while (curr != str.npos)
    {
        next = str.find_first_of(seps, curr);
        if (next != str.npos || curr != str.npos)
        {
            size_t last = (next==str.npos) ? str.npos : (next - curr);
            string strTok(str, curr, last);
            strVec.push_back(strTok);
        }
        curr = str.find_first_not_of(seps, next);
        if (next != str.npos || curr != str.npos)
        {
            size_t last = (curr==str.npos) ? str.npos : (curr - next);
            string strDelim(str, next, last);
//            strVec.push_back(strDelim);
            string::iterator it(strDelim.begin());
            for ( ; it != strDelim.end(); ++it)
                strVec.push_back(string(1, *it));
        }
    }

    cout << "(curr,next)=" << (curr==str.npos?0:curr)
         << "," << (next==str.npos?0:next) << endl;
}

void Parser::trimSpace(PairIter& pi, bool stripComments)
{
    if (stripComments)
    {
        size_t comment = pi.get().find("//");
        if (comment != string::npos)
        {
            pi.set(pi.begin(), pi.begin() + comment);
        }
    }

    string::const_iterator it = pi.begin();
    for ( ; it != pi.end(); ++it)
    {
        if (*it != ' ' && *it != '\t' && *it != '\n')
            break;
    }
    if (it == pi.end())
    {
        pi.set(it);
        return;
    }

    string::const_iterator ite = pi.end() - 1;
    for ( ; ite != it; --ite)
    {
        if (*ite != ' ' && *ite != '\t' && *ite != '\n')
            break;
    }
    pi.set(it, ite+1);
}

std::string Parser::trimSpace(const std::string& str)
{
    PairIter pi(str);
    trimSpace(pi, false);
    return pi.get();
}

std::vector<string> Parser::splitString(const std::string& str)
{
    std::vector<string> v;
    if (str.empty()) return v;

    size_t last = 0;
    size_t eol = 0;
    while (eol != string::npos)
    {
        eol = str.find('\n', last);
        v.push_back(str.substr(last, eol));
        last = eol + 1;
    }

    return v;
}
