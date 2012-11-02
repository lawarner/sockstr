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

#ifndef _Serializer_H_INCLUDED_
#define _Serializer_H_INCLUDED_
//
//

//
// INCLUDE FILES
//
#include <sstream>


namespace ipctest
{

//
// FORWARD CLASS DECLARATIONS
//
class TestBase;

//
// TYPE DEFINITIONS
//

//
// CLASS DEFINITIONS
//
class Serializer
{
public:
    Serializer(TestBase* testBase);
    ~Serializer();

    enum Section
    {
        UnknownSection,
        HeaderSection,
        MessageSection,
        CommandSection
    };

    bool serialize(const std::string& fileName);
    bool deserialize(const std::string& fileName);
    static std::string encodeString(const std::string& str);

public:	// Callback handlers for xml parsing
    void deserialStartTag(const char* el, const char** attr);
    void deserialEndTag(const char* el);
    void deserialCharData(const char *txt, int txtlen);
    void deserialPI(const char *target, const char *pidata);

private:
    void startCommand(const char** attr);

private:
	// buffer for deserialize xml parsing callbacks
	std::ostringstream& deserialData_;
    TestBase* testBase_;
    int parseLevel_;
    Section section_;
    Message* commandMessage_;
    std::string messageName_;
    Params commandParams_;
    Params fieldParams_;
    Params messageFields_;

private:
    Serializer(const Serializer&);	// disable copy constructor
    Serializer& operator=(const Serializer& rSource);	// disable assignment operator
};

}  // namespace ipctest

#endif
