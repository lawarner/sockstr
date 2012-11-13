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

// echoserver.cpp
//
// This example spawns a server socket thread that accepts client connections.
// Then it reads messages from client and echoes them back.

#include <cerrno>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#include <sockstr/Socket.h>
using namespace sockstr;
using namespace std;


bool exit_server = false;

struct Params
{
    int port;
    bool binary;
    Stream* clientSock;
};


void* client_process(void* args)
{
    Params* params = (Params*) args;
    Stream* clientSock = params->clientSock;
    cout << "Client process started." << endl;
    while (!exit_server && clientSock->queryStatus() == SC_OK)
    {
        if (params->binary)
        {
            char buf[256];
            int sz = clientSock->read(buf, sizeof(buf));
            if (sz > 0)
            {
                cout << "Echo " << sz << " bytes." << endl;
                clientSock->write(buf, sz);
            }
        }
        else
        {
            string strbuf;
            clientSock->read(strbuf, EOF);
            *clientSock << strbuf;
        }
    }

    clientSock->close();
    delete clientSock;

    cout << "Client exiting..." << endl;
    return 0;
}


void* server_process(void* args)
{
    void* ret = (void*) 2;
    Params* params = (Params*) args;
    cout << "Server connecting to port " << params->port << endl;

    Socket sock;
    SocketAddr saddr(0, params->port);
    if (!sock.open(saddr, Socket::modeReadWrite))
    {
        cout << "Error opening server socket: " << errno << endl;
        return ret;
    }

    while (sock.queryStatus() == SC_OK)
    {
        Stream* clientSock = sock.listen();
        if (clientSock)
        {
            Params clientParams = *params;
            clientParams.clientSock = clientSock;
            
            pthread_t tid;
            pthread_create(&tid, NULL, client_process, &clientParams);
        }
    }

    sock.close();
    return 0;
}


int main(int argc, char* argv[])
{
    Params params = {
        4321,
        true,
        0
    };

    int opt;
    while ((opt = getopt(argc, argv, "b")) != -1)
    {
        switch (opt)
        {
        case 'b':
            cout << "Using block copy" << endl;
            params.binary = true;
            break;
        default:
            cout << "Usage:  echoserver [ -b ] <port>" << endl;
            return 1;
        }
    }

    if (optind < argc)
        params.port = atoi(argv[optind]);

    server_process(&params);

//    void* res;
//    pthread_join(tid, &res);

    return 0;
}
