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
#include <pthread.h>
#include <sockstr/sstypes.h>
#include <sockstr/Socket.h>
using namespace sockstr;
using namespace std;

struct Params
{
    int port;
    string fileName;
};

void* server_process(void* args)
{
    void* ret = (void*) 2;
    Params* params = (Params*) args;
    cout << "Server connecting to port " << params->port << endl;
    string fileName = params->fileName + ".bak";
    cout << "Writing to file " << fileName << endl;

    Socket sock;
    SocketAddr saddr(0, params->port);
    if (!sock.open(saddr, Socket::modeReadWrite))
    {
        cout << "Error opening server socket: " << errno << endl;
        return ret;
    }

    Stream* clientSock = sock.listen();
    if (clientSock)
    {
        string strbuf;

#if 1
        clientSock->read(strbuf);
        cout << "Response: " << strbuf << endl;
#else
        *clientSock >> strbuf;
        while (clientSock->queryStatus() == SC_OK)
        {
             cout << "Response: " << strbuf << endl;
            *clientSock >> strbuf;
        }
#endif

        clientSock->close();
        delete clientSock;
    }

    sock.close();
    return 0;
}


void* client_process(void* args)
{
    void* ret = (void*) 2;
    Params* params = (Params*) args;
    cout << "Client process started, port " <<  params->port << endl;
    cout << "Reading from file " << params->fileName << endl;

    Socket sock;
    SocketAddr saddr("localhost", params->port);
    if (!sock.open(saddr, Socket::modeReadWrite))
    {
        cout << "Error opening client socket: " << errno << endl;
        return ret;
    }


    ifstream ifile(params->fileName.c_str());
    if (!ifile.is_open())
    {
        cout << "Could not open file " << params->fileName << endl;
        return ret;
    }

    string contents((istreambuf_iterator<char>(ifile)),
                    istreambuf_iterator<char>());
    ifile.close();

//    cout << "File contents:\n" << contents << endl;

    sock << contents << endl;

    sock.close();

    return 0;
}


int main(int argc, const char* argv[])
{
    Params params = { 4321, "" };
    if (argc > 1)
        params.fileName = argv[1];
    else
    {
        cout << "Usage:  filecopy <filename>" << endl;
        return 1;
    }

    pthread_t tid;
    pthread_create(&tid, NULL, server_process, &params);

    client_process(&params);

    void* res;
    pthread_join(tid, &res);

    return 0;
}
