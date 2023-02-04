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

// simplest.cpp
//
// This is the simplest standalone example of using sockstr for both client 
// and server sockets.

#include <cerrno>
#include <iostream>

#include <sockstr/Socket.h>
#include <sockstr/ThreadHandler.h>
using namespace sockstr;
using namespace std;

struct Params
{
    int port;
};


class ServerThreadHandler : public ThreadHandler<Params*, void*> {
public:
    ServerThreadHandler(Params* params) { setData(params); }

    virtual void* handle(Params* params);
};

void* ServerThreadHandler::handle(Params* params) {
    void* ret = (void*) 2;
    cout << "Server connecting to port " << params->port << endl;

    Socket sock;
    SocketAddr saddr(params->port);
    if (!sock.open(saddr, Socket::modeReadWrite))
    {
        cout << "Error opening server socket: " << errno << endl;
        return ret;
    }

    cout << "Listen for client connection" << endl;
    Stream* clientSock = sock.listen();
    if (clientSock)
    {
        std::string strbuf;

        while (clientSock->queryStatus() == SC_OK) {
            *clientSock >> strbuf;
             cout << "Response: " << strbuf << endl;
             if (strbuf.empty() || strbuf == "<EOM>") {
                 break;
             }
        }
        *clientSock << "I got your message <EOM>" << endl;

        clientSock->close();
        delete clientSock;
    }

    sock.close();
    return 0;
}


void* client_process(void* args) {
    void* ret = (void*) 2;
    Params* params = (Params*) args;
    cout << "Client process started." << endl;

    Socket sock;
    SocketAddr saddr("localhost", params->port);
    if (!sock.open(saddr, Socket::modeReadWrite)) {
        cout << "Error opening client socket: " << errno << endl;
        return ret;
    }

    std::string str("Sending a test string.\n<EOM>");
    sock << str << endl;
    cout << "String sent to server, try reading" << endl;
    while (sock.good()) {
      sock >> str;
      cout << "Received from server: \"" << str << "\"" << endl;
      if (str == "<EOM>") {
        break;
      }
    }
    sock.close();

    return 0;
}


int main(int argc, const char* argv[]) {
    Params params = { 4321 };

    ServerThreadHandler server(&params);
    ThreadManager::create<Params*, void*>(&server);

    client_process(&params);

    server.wait();

    return 0;
}
