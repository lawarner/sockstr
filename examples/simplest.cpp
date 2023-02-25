/*
   Copyright (C) 2012, 2013, 2023
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

#include <sockstr/Socket.h>
#include <cerrno>
#include <iostream>
#include <thread>
using namespace sockstr;
using std::cout;
using std::endl;

struct Params {
    int port;
};


void server_handler(const Params* params) {
    cout << "Server connecting to port " << params->port << endl;

    Socket sock;
    SocketAddr saddr(params->port);
    if (!sock.open(saddr, Socket::modeReadWrite)) {
        cout << "Error opening server socket: " << errno << endl;
        return;
    }

    cout << "Listen for client connection" << endl;
    Stream* clientSock = sock.listen();
    if (clientSock) {
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
}


bool client_process(const Params* params) {
    cout << "Client process started." << endl;

    Socket sock;
    SocketAddr saddr("localhost", params->port);
    if (!sock.open(saddr, Socket::modeReadWrite)) {
        cout << "Error opening client socket: " << errno << endl;
        return false;
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
    return true;
}


int main(int argc, const char* argv[]) {
    Params params = { 4321 };

    auto server = std::thread(server_handler, &params);

    bool ret = client_process(&params);
    cout << "Client finished " << (ret ? "ok" : "with error") << endl;

    // wait for server thread to end
    if (server.joinable()) {
        server.join();
    }
    return 0;
}
