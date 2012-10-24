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

// ipctest.cpp
//
// This is a testing tool used to create and execute various collections
// of tests.  The underlying IPC mechanism uses the sockstr library.

#include <cerrno>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <sockstr/sstypes.h>
#include <sockstr/Socket.h>
#include "Parser.h"

using namespace sockstr;
using namespace std;
using namespace ipctest;

struct TParams
{
    string ipcDefsFile;
};

void* server_process(void* args)
{
    void* ret = (void*) 2;
//    TParams* params = (TParams*) args;

    Socket sock;
    SocketAddr saddr(0, 4321);
    if (!sock.open(saddr, Socket::modeReadWrite))
    {
        cout << "Error opening server socket: " << errno << endl;
        return ret;
    }

    Stream* clientSock = sock.listen();
    if (clientSock)
    {
        string strbuf;

//        clientSock->read(strbuf);
        *clientSock >> strbuf;
        while (clientSock->queryStatus() == SC_OK)
        {
             cout << "Response: " << strbuf << endl;
            *clientSock >> strbuf;
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
    TParams* params = (TParams*) args;
    cout << "Client process started, reading from " << params->ipcDefsFile << endl;

    ifstream ifile(params->ipcDefsFile.c_str());
    if (!ifile.is_open())
    {
        cerr << "Could not open IPC definitions file " << params->ipcDefsFile << endl;
        return ret;
    }

    string strIpcDefs((istreambuf_iterator<char>(ifile)),
                      istreambuf_iterator<char>());
    ifile.close();

    MessageList msgList;
    Parser parse;
    if (!parse.stringToMessageList(strIpcDefs, msgList))
    {
        cerr << "Error parsing message list" << endl;
        return ret;
    }

    cout << " * * *  MESSAGE LIST  * * *" << endl;
    MessageList::iterator it;
    for (it = msgList.begin(); it != msgList.end(); ++it)
    {
        cout << (*it)->getOrdinal() << " " << (*it)->getName()
             << ", size=" << (*it)->getSize() << endl;
    }

    return 0;
}


int main(int argc, const char* argv[])
{
    TParams params;
    if (argc > 1)
        params.ipcDefsFile = argv[1];
    else
    {
        cout << "Usage: ipctest <ipcdef_filename>" << endl;
        return 1;
    }

//    pthread_t tid;
//    pthread_create(&tid, NULL, server_process, &params);

    client_process(&params);

//    void* res;
//    pthread_join(tid, &res);

    return 0;
}
