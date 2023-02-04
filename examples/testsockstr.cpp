/*
   Copyright (C) 2012 - 2022
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

// testsocklib.cpp
//
//  Unit test program for socket library routines
//
#ifdef WIN32
#include <WS2tcpip.h>
#endif

#include <iostream>
#include <cerrno>
#include <cstdio>
#include <cstring>

//#include <sockstr/IPAddress.h>
#include <sockstr/Socket.h>

using namespace sockstr;
using std::cout;
using std::endl;

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Error: Usage:  testsocklib hostname filename [summary]" << endl;
        return(1);
    }
    cout << "testsocklib: program started." << endl;

    bool isSummary = (argc > 3);
    std::string hostname = argv[1];
    std::string filename = argv[2];
    if (filename[0] != '/') {
        filename.insert(0, 1, '/');
    }
    std::string hostport = hostname;
    if (hostport.find(':') == hostport.npos) {
        hostport += ":80";
    }
    cout << "Get address of host " << hostport << endl;

    // IPAddress ipaddr(hostname.c_str());
    // printf("Netaddress is %x, string value is %s\n",
    //        ipaddr.netAddress(), (const char*) ipaddr);

    Socket sock;
    if (!sock.open(hostport.c_str(), Socket::modeReadWrite)) {
        cout << "Error opening socket: " << errno << endl;
        return(2);
    }
    cout << "Socket open at " << (const char *) sock << endl;

    std::string http_get = "GET " + filename;
    // Add HTTP headers
    http_get += " HTTP/1.1\r\nHost: " + hostname + "\r\n";
    http_get += "Accept: */*\r\n";

    sock << http_get << endl;

    char buf[1024] = "";
    int inLen;
    int totalLen = 0;
    while ((inLen = sock.read(buf, sizeof(buf))) > 0) {
        if (isSummary)
            cout << "+=+=+=+=+ Read from socket " << inLen << " characters" << endl;
        else
            cout << std::string(buf, inLen);
        totalLen += inLen;
    }
    if (isSummary) {
        cout << "+=+=+=+=+ Total read from socket is " 
                  << totalLen << " characters" << endl;
    }
    cout << endl;

    sock.close();
    return(0);
}


#ifdef WIN32
#include "stdafx.h"
int _tmain(int argc, _TCHAR* targv[])
{
	char **argv = new char* [argc];
	for (int i = 0; i < argc; i++)
	{
		int sz = _tcslen(targv[i]) + 1;
		argv[i] = new char [sz];
		wcstombs(argv[i], targv[i], sz);
	}

	return main(argc, argv);
}
#endif
