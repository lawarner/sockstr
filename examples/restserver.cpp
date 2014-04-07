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

// filecopy.cpp
//
// This example spawns 2 threads that talk over a socket.
// The first thread reads a file and sends it on the socket.
// The second thread reads the socket and writes the contents to a file.

#include <cerrno>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <cstring>
#include <unistd.h>

#include <sockstr/HttpHelpers.h>
#include <sockstr/HttpStream.h>
#include <sockstr/Socket.h>
#include <sockstr/ThreadHandler.h>
using namespace sockstr;
using namespace std;

struct Params
{
    Params(int lPort, Stream* lClient) : port(lPort), client(lClient) { }
    int port;
    Stream* client;
};


class RequestThreadHandler : public ThreadHandler<Params*, void*>
{
public:
    RequestThreadHandler(Params* params) { setData(params); }

    virtual void* handle(Params* params);
};

void* RequestThreadHandler::handle(Params* params)
{
    cout << "Server thread connected to port " << params->port << endl;

    Stream* sock = params->client;
    SocketAddr saddr(0, params->port);

    char buf[256];
    int sz = sock->read(buf, sizeof(buf));
    HttpStatus httpStatus;
    while (sock->queryStatus() == SC_OK)
    {
//           ofile.write(buf, sz);
        sz = sock->read(buf, sizeof(buf));
        *sock << httpStatus.statusLine() << "\r\n"
              << "{ jsondata: { a: 1, b: 2 } }\r" << endl;
    }

    sock->close();
    delete sock;

    return 0;
}


int main(int argc, char* argv[])
{
    int port = 4321;

    int opt;
    while ((opt = getopt(argc, argv, "D")) != -1)
    {
        switch (opt)
        {
        case 'D':
            cout << "Debugging output" << endl;
            break;
        default:
            cout << "Usage:  restserver [ -D ] [ port ]" << endl;
            return 1;
        }
    }

    if (optind < argc)
        port = atoi(argv[optind]);
    else
    {
        cout << "Usage:  filecopy [ -D ] [ port ]" << endl;
        return 1;
    }

    cout << "Server connecting to port " << port << endl;

    Socket sock;
    SocketAddr saddr(0, port);
    if (!sock.open(saddr, Socket::modeReadWrite))
    {
        cout << "Error opening socket: "
             << errno << ": " << strerror(errno) << endl;
        return 2;
    }

    do {
        Stream* clientSock = sock.listen();
        if (clientSock)
        {
            Params params(port, clientSock);
            RequestThreadHandler server(&params);
            ThreadManager::create<Params*, void*>(&server);
        }
    } while (true);

    //server.wait();	// wait for server thread to end

    return 0;
}
