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
#include <unistd.h>
#include <sockstr/sstypes.h>
#include <sockstr/Socket.h>
using namespace sockstr;
using namespace std;

struct Params
{
    int port;
    bool binary;
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
        ifstream ifile(fileName.c_str());
        if (ifile.is_open())
        {
            cout << "Output file already exists.  Not overwriting." << endl;
            return ret;
        }
        ofstream ofile(fileName.c_str());

        if (params->binary)
        {
            char buf[256];
            int sz = clientSock->read(buf, sizeof(buf));
            while (clientSock->queryStatus() == SC_OK)
            {
                ofile.write(buf, sz);
                sz = clientSock->read(buf, sizeof(buf));
            }
        }
        else
        {
            string strbuf;
            clientSock->read(strbuf, EOF);
            ofile << strbuf;
        }

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

    sock << contents;

    sock.close();

    return 0;
}


int main(int argc, char* argv[])
{
    Params params = {
        4321,
        false,
        ""
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
            cout << "Usage:  filecopy [ -b ] <filename>" << endl;
            return 1;
        }
    }

    if (optind < argc)
        params.fileName = argv[optind];
    else
    {
        cout << "Usage:  filecopy [ -b ] <filename>" << endl;
        return 1;
    }

    pthread_t tid;
    pthread_create(&tid, NULL, server_process, &params);

    client_process(&params);

    void* res;
    pthread_join(tid, &res);

    return 0;
}
