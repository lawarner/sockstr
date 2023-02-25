/*
   Copyright (C) 2012, 2023
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

#include <sockstr/Socket.h>

#include <cerrno>
#include <fstream>
#include <iostream>
#include <thread>
#include <unistd.h>

using namespace sockstr;
using std::cout;
using std::endl;

struct Params {
    int port;
    bool binary;
    std::string fileName;
};


void server_handler(const Params* params) {
    cout << "Server connecting to port " << params->port << endl;
    std::string fileName = params->fileName + ".bak";
    cout << "Writing to file " << fileName << endl;

    Socket sock;
    SocketAddr saddr(params->port);
    if (!sock.open(saddr, Socket::modeReadWrite)) {
        cout << "Error opening server socket: " << errno << endl;
        return;
    }

    Stream* clientSock = sock.listen();
    if (clientSock) {
        std::ifstream ifile(fileName.c_str());
        if (ifile.is_open()) {
            cout << "Output file already exists.  Not overwriting." << endl;
            return;
        }
        std::ofstream ofile(fileName.c_str());

        if (params->binary) {
            char buf[256];
            int sz = clientSock->read(buf, sizeof(buf));
            while (clientSock->queryStatus() == SC_OK) {
                ofile.write(buf, sz);
                sz = clientSock->read(buf, sizeof(buf));
            }
        } else {
            std::string strbuf;
            clientSock->read(strbuf, EOF);
            ofile << strbuf;
        }

        clientSock->close();
        delete clientSock;
    }

    sock.close();
}


bool client_process(const Params* params) {
    cout << "Client process started, port " <<  params->port << endl;
    cout << "Reading from file " << params->fileName << endl;

    Socket sock;
    SocketAddr saddr("localhost", params->port);
    if (!sock.open(saddr, Socket::modeReadWrite))
    {
        cout << "Error opening client socket: " << errno << endl;
        return false;
    }


    std::ifstream ifile(params->fileName.c_str());
    if (!ifile.is_open()) {
        cout << "Could not open file " << params->fileName << endl;
        return false;
    }

    std::string contents((std::istreambuf_iterator<char>(ifile)),
                         std::istreambuf_iterator<char>());
    ifile.close();

//    cout << "File contents:\n" << contents << endl;

    sock << contents;

    sock.close();

    return true;
}


int main(int argc, char* argv[]) {
    Params params = {
        4321,
        false,
        ""
    };

    int opt;
    while ((opt = getopt(argc, argv, "b")) != -1) {
        switch (opt) {
        case 'b':
            cout << "Using block copy" << endl;
            params.binary = true;
            break;
        default:
            cout << "Usage:  filecopy [ -b ] <filename>" << endl;
            return 1;
        }
    }

    if (optind < argc) {
        params.fileName = argv[optind];
    } else {
        cout << "Usage:  filecopy [ -b ] <filename>" << endl;
        return 1;
    }

    auto server = std::thread(server_handler, &params);

    bool ret = client_process(&params);
    cout << "Client finished " << (ret ? "ok" : "with error") << endl;

    // wait for server thread to end
    if (server.joinable()) {
        server.join();
    }
    return 0;
}
