/*
   Copyright (C) 2013
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

// httptest.cpp
//
//  Test program for the HttpStream class
//

#include <algorithm>
#include <iostream>
#include <sstream>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sockstr/HttpStream.h>
#include <sockstr/OAuth.h>

using namespace sockstr;
using namespace std;


//static const char* DEFAULT_URL = "http://www.stackoverflow.com:80";
static const char* DEFAULT_URL = "localhost:4321";
static const char* DEFAULT_API = "/";


int main(int argc, const char *argv[])
{
    cout << argv[0] << ": program started." << endl;

    string restApi = (argc > 1) ? argv[1] : DEFAULT_API;
    string url = (argc > 2) ? argv[2] : DEFAULT_URL;
    cout << "Calling '" << restApi << "' at url=" << url << endl;

    HttpStream http;
    if (!http.open(url.c_str(), Socket::modeReadWrite))
    {
        cout << "Error opening socket: "
             << errno << ": " << strerror(errno) << endl;
        return(2);
    }

    string strhost((const char *) http);
    strhost = strhost.substr(0, strhost.find_first_of(':'));
    cout << "Client socket open at " << strhost << endl;

    TimestampEncoder dateTime(true);

    http.loadDefaultHeaders();
    http.addHeader("Host", strhost);
    http.addHeader("Date", dateTime.toString());

    char buf[512] = "";
    string strbuf;
    string headerbuf;
    string contentbuf;
    int crlfpos = -1;

    int inlen = http.get(restApi, contentbuf, headerbuf);
    cout << "====== Read header " << headerbuf.length() << " bytes from socket:"
         << endl << headerbuf << endl;
    cout << "====== Read content " << contentbuf.length() << " bytes from socket:"
         << endl << contentbuf << endl;

    http.close();

    return(0);
}
