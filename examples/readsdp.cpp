/*
   Copyright (C) 2012, 2013
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

// readudp.cpp
//
//  Unit test program for socket library routines to read packets from a udp stream
//
#ifdef WIN32
#include <WS2tcpip.h>
#endif

#include <iostream>
#include <sstream>
#include <cerrno>
#include <cstdio>
#include <cstring>

#include <sockstr/Socket.h>
#include <sockstr/SocketAddr.h>

using namespace sockstr;


std::string doHeaders(int seq)
{
    std::ostringstream oss(" RTSP/1.0\r\nCSeq: ");
    oss << seq << "\r\nUser-Agent: LibVLC/2.0.8 (LIVE555 Streaming Media v2011.12.2 3)\r\n\r";
    return oss.str();
}

int main(int argc, char *argv[])
{
#if 1	// for debugging
    // std::string hostname = "192.168.0.7";
    // int port = 8000;
    // std::string filename = "/pi.sdp";
    std::string hostname = "66.207.140.116";
    int port = 80;
    std::string filename = "/slcn_sports.sdp";
    std::string sdp_path = "rtsp://66.207.140.116:80/slcn_sports.sdp";
    bool isSummary = false;
#else
    if (argc < 1)
    {
        std::cerr << "Error: Usage:  readudp hostname" << std::endl;
        return(1);
    }

    bool isSummary = (argc > 3);

    std::string hostname = argv[1];
    std::string filename = argv[2];
    if (filename[0] != '/')
        filename.insert(0, 1, '/');
    std::string hostport = hostname;
    if (hostport.find(':') == hostport.npos)
        hostport += ":80";
#endif
    std::cout << "Prepare to connect to host " << hostname << " udp port " << port << std::endl;

    Socket sock;
    SocketAddr saddr(hostname.c_str(), port);
    if (!sock.open(saddr, Socket::modeReadWrite))
    {
        std::cout << "Error opening socket: " 
                  << errno << ": " << strerror(errno) << std::endl;
        return(2);
    }
    std::cout << "Socket open at " << (const char *) sock << std::endl;

    sock << "OPTIONS " << sdp_path << doHeaders(2) << std::endl;

    char buf[1500] = "";
    int inLen;
    int totalLen = 0;
    while ((inLen = sock.read(buf, sizeof(buf))) > 0)
    {
        totalLen += inLen;
        if (isSummary)
            std::cout << "+=+=+=+=+ Read from socket " << inLen << " characters" << std::endl;
        else
        {
            std::string strout = std::string(buf, inLen);
            std::cout << strout;
            if (strout.find("\r\n\r\n")) break;
        }
    }
    if (isSummary)
        std::cout << "+=+=+=+=+ Total read from socket is " 
                  << totalLen << " characters" << std::endl;
    std::cout << std::endl;

    sock << "DESCRIBE " << sdp_path << doHeaders(3) << std::endl;
    totalLen = 0;
    bool header = false;
    while ((inLen = sock.read(buf, sizeof(buf))) > 0)
    {
        totalLen += inLen;
        if (isSummary)
            std::cout << "+=+=+=+=+ Read from socket " << inLen << " characters" << std::endl;
        else
        {
            std::string strout = std::string(buf, inLen);
            std::cout << strout;
            if (strout.find("\r\n\r\n"))
            {
                if (header)
                    break;
                else
                    header = true;
            }
        }
    }
    

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
