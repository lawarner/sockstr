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

#include <sockstr/IPAddress.h>
#include <sockstr/Socket.h>

using namespace sockstr;


int main(int argc, char *argv[])
{
#if 0	// for debugging in vs2010
	std::string hostname = "stackoverflow.com";
    std::string filename = "/index.html";
	bool isSummary = false;
#else
    if (argc < 3)
    {
        std::cerr << "Error: Usage:  testsocklib hostname filename [summary]" << std::endl;
        return(1);
    }
    std::cout << "testsocklib: program started.\n";

    bool isSummary = (argc > 3);

    std::string hostname = argv[1];
    std::string filename = argv[2];
    if (filename[0] != '/')
        filename.insert(0, 1, '/');
#endif
	std::string hostport = hostname + ":80";
    std::cout << "Get address of host " << hostport << std::endl;


    IPAddress ipaddr(hostname.c_str());
    printf("Netaddress is %x, string value is %s\n",
           ipaddr.netAddress(), (const char*) ipaddr);

    Socket sock;
    if (!sock.open(hostport.c_str(), Socket::modeReadWrite))
    {
        std::cout << "Error opening socket: " << errno << std::endl;
        return(2);
    }
    std::cout << "Socket open at " << (const char *) sock << std::endl;

    std::string http_get = "GET " + filename;
#if 1							// Add HTTP headers
	http_get += "HTTP/1.1\r\nHost: " + hostname + "\r\n";
    http_get += "Accept: */*\r\n";
#endif
	http_get += "\r\n";

    sock.write(http_get);

    char buf[1024] = "";
    int inLen;
    int totalLen = 0;
    while ((inLen = sock.read(buf, sizeof(buf))) > 0)
    {
        if (isSummary)
            std::cout << "+=+=+=+=+ Read from socket " << inLen << " characters" << std::endl;
        else
            std::cout << std::string(buf, inLen);
        totalLen += inLen;
    }
    if (isSummary)
        std::cout << "+=+=+=+=+ Total read from socket is " 
                  << totalLen << " characters" << std::endl;
    std::cout << std::endl;

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
