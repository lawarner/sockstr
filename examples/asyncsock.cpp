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

// asyncsock.cpp
//
//  Unit test program for socket library routines
//  This tests asynchronous mode for Sockets using callbacks.
//
#ifdef WIN32
#include <WS2tcpip.h>
#endif

#include <iostream>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <unistd.h>

#include <sockstr/IPAddress.h>
#include <sockstr/Socket.h>
using namespace sockstr;

bool bImDone = false;


void readSockData(DWORD dw, void* data)
{
    std::cout << "got some data " << dw << " " << std::string((char*)data, dw) << std::endl;
    if (dw < 1024) bImDone = true;
}


int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Error: Usage:  asyncsock hostname filename [summary]" << std::endl;
        return(1);
    }
    std::cout << "asyncsock: program started.\n";

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
    Socket sock;
    if (!sock.open(hostport.c_str(), Socket::modeReadWrite))
//                   Socket::modeAsyncSocket | Socket::modeReadWrite))
    {
        std::cout << "Error opening socket: "
                  << errno << ": " << strerror(errno) << std::endl;
        return(2);
    }
    std::cout << "Socket open at " << (const char *) sock << std::endl;

    sock.registerCallback(readSockData);

    std::string http_get = "GET " + filename;
    // Add HTTP headers
    http_get += " HTTP/1.1\r\nHost: " + hostname + "\r\n";
    http_get += "Accept: */*\r\n\r";

	sock << http_get << std::endl;	// Send the request synchronously

    sock.setAsyncMode(true);

    char buf[1024] = "";
    int inLen;
    int totalLen = 0;
    while (sock.good())
    {
        inLen = sock.read(buf, sizeof(buf));
        if (inLen <= 0 && bImDone) break;

        if (isSummary)
            std::cout << "+=+=+=+=+ Read from socket " << inLen << " characters" << std::endl;
        else
            std::cout << "+= " << std::string(buf, inLen) << std::endl;
        totalLen += inLen;
        sleep(1);
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
