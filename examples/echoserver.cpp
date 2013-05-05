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

// echoserver.cpp
//
// This example spawns a server socket thread that accepts client connections.
// Then it reads messages from client and optionally echoes them back.
//
// This program relies on posix threads and is only known to work on 
// linux for now.

#include <cerrno>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include <sockstr/Socket.h>
using namespace sockstr;
using namespace std;


bool exit_server = false;
Socket serverSock;

// Params is the structure passed between threads
struct Params
{
    Params(int _port, bool _binary, bool _echo, Stream* _sock = 0)
        : port(_port), binary(_binary), echo(_echo), clientSock(_sock) { }
    Params(Params* p) : port(p->port), binary(p->binary), echo(p->echo), 
                        clientSock(p->clientSock) { }

    int port;
    bool binary;
    bool echo;
    Stream* clientSock;
};


void* client_process(void* args)
{
    cout << "Client process started." << endl;
    Params* params = static_cast<Params*>(args);
    Stream* clientSock = params->clientSock;
    int totalRead = 0;

    while (!exit_server && clientSock->good())
    {
        if (params->binary)
        {
            char buf[512];
            int sz = clientSock->read(buf, sizeof(buf));
            if (sz > 0)
            {
                if (params->echo)
                    cout << string(buf, sz) << endl;
                else
                    cout << "Echoing " << sz << " bytes." << endl;

                clientSock->write(buf, sz);
                totalRead += sz;
            }
        }
        else
        {
            string strbuf;
            clientSock->read(strbuf, EOF);
            *clientSock << strbuf;
        }
    }

    if (clientSock->good())
        clientSock->close();
    delete clientSock;
    delete params;

    cout << "Client exiting... total bytes read=" << totalRead << endl;
    return 0;
}


void* server_process(void* args)
{
    void* ret = (void*) 2;
    Params* params = (Params*) args;
    cout << "Server connecting to port " << params->port << endl;

    SocketAddr saddr(0, params->port);
    if (!serverSock.open(saddr, Socket::modeReadWrite))
    {
        cout << "Error opening server socket: " << errno << endl;
        return ret;
    }

    while (!exit_server && serverSock.good())
    {
        Stream* clientSock = serverSock.listen();
        if (clientSock && clientSock->good())
        {
            Params *clientParams = new Params(params);
            clientParams->clientSock = clientSock;
            
            pthread_t tid;
            pthread_create(&tid, NULL, client_process, clientParams);
        }
    }

    if (serverSock.good())
        serverSock.close();
    return 0;
}

// Signal handler
void quitit(int sig)
{
    cout << endl << "Exiting...";
    // Exit as cleanly as possible
    exit_server = true;
    serverSock.close();
    cout << endl;
}



int main(int argc, char* argv[])
{
    Params params(4321, true, true);

    int opt;
    while ((opt = getopt(argc, argv, "as")) != -1)
    {
        switch (opt)
        {
        case 'a':
            params.binary = false;
            break;
        case 's':
            params.echo = false;
            break;
        default:
            cout << "Usage:  echoserver [ -as ] <port>" << endl
                 << "          -a for string reads on socket" << endl
                 << "          -s = only display summary on stdout" << endl;
            return 1;
        }
    }

    if (optind < argc)
        params.port = atoi(argv[optind]);

    
    cout << "Using " << (params.binary ? "block" : "string") << " copy, "
         << (params.echo ? "echoing" : "not echoing") << " contents." << endl;

    signal(SIGINT, quitit);
    signal(SIGQUIT, quitit);

    server_process(&params);

//    void* res;
//    pthread_join(tid, &res);

    return 0;
}
