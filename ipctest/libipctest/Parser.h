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

#ifndef _PARSER_H_INCLUDED_
#define _PARSER_H_INCLUDED_
//
//

//
// INCLUDE FILES
//
#include <string>
#include <vector>
#include "Message.h"


namespace ipctest
{

//
// MACRO DEFINITIONS
//
#ifndef DllExport
#define DllExport
#endif

static const char* TOKEN_SEPERATORS = " \t\n\r;:()/#*";

//
// FORWARD CLASS DECLARATIONS
//

//
// TYPE DEFINITIONS
//

//
// CLASS DEFINITIONS
//
class PairIter
{
public:
    PairIter(void) { }
	PairIter(std::string::const_iterator itBegin, std::string::const_iterator itEnd)
        : begin_(itBegin), end_(itEnd) { }
    PairIter(const std::string& str)
        : begin_(str.begin()), end_(str.end()) { }


    const std::string::const_iterator& begin() const { return begin_; }
    const std::string::const_iterator& end()   const { return end_; }
    void get(std::string::const_iterator& itBegin, std::string::const_iterator& itEnd) const
    {
        itBegin = begin_;
        itEnd = end_;
    }
    std::string get() { return std::string(begin_, end_); }
    void set(const std::string::const_iterator& itBegin)
    {
        begin_ = itBegin;
    }
    void set(const std::string::const_iterator& itBegin,
             const std::string::const_iterator& itEnd)
    {
        begin_ = itBegin;
        end_ = itEnd;
    }
private:
    std::string::const_iterator begin_;
    std::string::const_iterator end_;
};

/*
  (a)    t1
       / |  \
    d1(  a   )

    t2  and(a,"b")
    |  \
   a,"b" )

*/
class PTNode
{
public:
    void addChild(PTNode* child) { children_.push_back(child); }
protected:
    std::vector<PTNode*> children_;
};
class PTDelimiter : public PTNode
{
    std::string open_;
    std::string close_;
};
class PTContent : public PTNode
{
    std::string content_;
};
class ParseTree : public PTNode
{
    
};


class Parser
{
public:
    Parser();
    virtual ~Parser();

    bool fileToMessageList(const std::string& fname, MessageList& ml);
    bool stringToMessageList(const std::string& str, MessageList& ml);

    Message* createMessage(const std::string& msgName, PairIter& inStr);

    static bool getLine(PairIter& inStr, PairIter& outLine, bool trim = true);
    static bool makeParseTree(const std::string& str, ParseTree& tree);
    static void splitTokens(const std::string& str, std::vector<std::string>& strVec,
        const char* seps = TOKEN_SEPERATORS);
    static void splitDelimitedTokens(const std::string& str, std::vector<std::string>& strVec,
        const char* seps = TOKEN_SEPERATORS);
    static void trimSpace(PairIter& pi, bool stripComments = true);
    static std::string trimSpace(const std::string& instr);
    static std::vector<std::string> splitString(const std::string& str);

private:
    Parser(const Parser&);	// disable copy constructor
    Parser& operator=(const Parser& rSource);	// disable assignment operator
};

}  // namespace ipctest

#endif
