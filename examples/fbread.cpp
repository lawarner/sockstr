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

// rbread.cpp
//
//  Gtk+ GUI application to browse facebook using the SockStr api
//

#include <algorithm>
#include <iostream>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sockstr/Socket.h>

using namespace sockstr;

static const char* FBURL = "http://graph.facebook.com:80";


int main(int argc, const char *argv[])
{
    std::cout << "fbread: program started.\n";
    if (argc < 2)
    {
        std::cerr << "Error: Usage:  fbread facebook_id" << std::endl;
        return(1);
    }

    std::string fbid = argv[1];
    std::string url = FBURL;

    std::cout << "Opening url=" << url << std::endl;

    Socket sock;
    if (!sock.open(url.c_str(), Socket::modeReadWrite))
    {
        std::cout << "Error opening socket: " << errno << std::endl;
        return(2);
    }

    std::cout << "Client socket open at " << (const char *) sock << std::endl;

    std::string http_get = "GET /" + fbid + " HTTP/1.1\r\n"
        + "Host: graph.facebook.com\r\n"
        + "Accept: */*\r\n"
        + "\r\n";
    sock.write(http_get.c_str(), http_get.length());

    std::string strbuf;
    std::string contentbuf;
    char buf[512] = "";
    int crlfpos = -1;
    int inlen;
    while ((inlen = sock.read(buf, sizeof(buf))) > 0)
    {
//        std::cout << "+=+=+=+=+ Read from socket " << inlen << " characters:" << std::endl
//                  << strbuf << std::endl;
        strbuf += std::string(buf, inlen);

        size_t ipos = strbuf.find("Content-Length: ");
        if (ipos != std::string::npos)
        {
            ipos += 16;         // skip past "Content-Length: "
            crlfpos = strbuf.substr(ipos).find("\n");
            int content_len = strtol(strbuf.substr(ipos, crlfpos).c_str(), NULL, 10);
            std::cout << " Content length is: " << content_len
                      << ".  snippet: " << strbuf.substr(ipos + crlfpos + 2, 10) << std::endl;
            contentbuf = strbuf.substr(ipos + crlfpos + 2);
            size_t bufsize = sizeof(buf);
            for (int idx = contentbuf.length(); idx < content_len; )
            {
                size_t seglen = content_len - idx + 1;
                int ilen = std::min(bufsize, seglen);
//                int ilen = std::min(sizeof(buf), (unsigned int)(content_len - idx + 1));
                inlen = sock.read(buf, ilen);
                if (inlen <= 0)
                    break;
                contentbuf += std::string(buf, inlen);
                std::cout << " +++buf: " << std::string(buf).substr(0,60) << "..." << std::endl;
                idx += inlen;
            }
            strbuf = contentbuf;
            break;
        }

        // End of JSON detected?
        if (strbuf.rfind("\"}") == (strbuf.length() - 2))
            std::cout << "End of JSON found." << std::endl;
    }
    std::cout << "======== Read from Socket:" << std::endl << strbuf << std::endl << std::endl;

    sock.close();

    return(0);
}
