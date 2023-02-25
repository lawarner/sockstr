/*
   Copyright (C) 2014, 2023
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

// restserver.cpp
//
// An example of a multi-threaded REST server.

#include <sockstr/HttpHelpers.h>
#include <sockstr/HttpStream.h>
#include <sockstr/Socket.h>

#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <thread>
#include <vector>
#include <unistd.h>
using namespace sockstr;
using std::cout;
using std::endl;


static bool debugOut = false;

struct Params {
    Params(int lPort, HttpServerStream* lClient) : port(lPort), client(lClient) { }
    int port;
    HttpServerStream* client;
};


void request_handler(Params* params) {
    HttpServerStream* sock = params->client;
    const char* hostport = *sock;
    cout << "Server thread connected to " << hostport << endl;
    sock->loadDefaultHeaders();

    char buf[1024];
    static const char* errorJson = "{ error: { text: \"Malformed request\" } }\r\n";

    if (sock->queryStatus() == SC_OK) {
        HttpServerStream::HttpFunction funct;
        std::string url;
        unsigned int sz = sock->request(buf, sizeof(buf), funct, url);
        if (sz == 0) {
            cout << "ERROR on http request" << endl;
            sock->response(errorJson, strlen(errorJson), "application/json", 404);
        } else {
            const char* funcName = sock->functionName(funct);
            cout << "HttpReq: " << funcName << " " << url << "." << endl;
            if (debugOut) {
                cout << std::string(buf, sz) << endl;
            }
            std::ostringstream someJson;
            someJson << "{ http: \"" << funcName << "\", url: \"" << url << "\" }\r\n";
            sock->response(someJson.str().c_str(), someJson.str().size(), "application/json");
        }
    }

    sock->close();
    delete sock;
    delete params;
}


int main(int argc, char* argv[]) {
    int port = 4321;
    int opt;
    while ((opt = getopt(argc, argv, "D")) != -1) {
        switch (opt) {
            case 'D':
                debugOut = true;
                break;
            default:
                cout << "Usage:  restserver [ -D ] [ port ]" << endl;
                return 1;
        }
    }

    if (optind < argc) {
        port = atoi(argv[optind]);
    }

    cout << "Server connecting to port " << port << endl;

    UrlParameterEncoder upe("My_good ness", "M&M's are\t great!;");
    cout << "UrlQuery: " << upe.toString() << endl;

    HttpServerStream sock;
    SocketAddr saddr(port);
    if (!sock.open(saddr, Socket::modeReadWrite)) {
        cout << "Error opening socket: "
             << errno << ": " << strerror(errno) << endl;
        return 2;
    }

    std::vector<std::thread> threads;
    do {
        HttpServerStream* clientSock = (HttpServerStream*) sock.listen();
        if (clientSock) {
            Params* params = new Params(port, clientSock);
            threads.emplace_back(request_handler, params);
        }
    } while (true);

    for (auto& thr : threads) {
        if (thr.joinable()) {
            thr.join();
        }
    }
    threads.clear();

    return 0;
}
