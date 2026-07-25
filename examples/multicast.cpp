/*
   Copyright (C) 2026
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

// multicast.cpp
//
// Example of sending and receiving multicast packets (datagrams) using UDP protocol.
// A thread is spawned for the multicast receiver while the main thread acts as the
// sender.
#include <sockstr/Socket.h>
#include <sockstr/SocketAddr.h>
#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>
using namespace sockstr;
using std::cout;
using std::endl;

std::string host;

void server() {
    cout << "Multicast receiver server is running" << endl;
    WORD port = 5000;

    SocketAddr sa(host, port, "udp");
    Socket sock;
    if (!sock.open(sa, Socket::modeReadWrite | Socket::modeCreate)) {
        cout << "Error opening receiver socket" << endl;
        return;
    }
    while (true) {
        cout << "Server going to block reading multicast datagram" << endl;
        char chbuf[200];
        auto num = sock.read(chbuf, sizeof(chbuf) - 1);
        if (num < 0) {
            cout << "Error reading socket, got " << num << endl;
            break;
        }
        chbuf[num] = '\0';
        std::string buf(chbuf);
        cout << "Server read " << num << " bytes: " << buf << endl;
        cout << "Socket is: " << (const char*)sock << endl;
        if (buf.starts_with("<EOF>")) {
            break;
        }
    }
    cout << "Multicast receiver server terminated" << endl;
}

int main(int argc, char* argv[]) {
  bool useIpv6 = (argc > 1 && strcmp(argv[1], "v4") == 0) ? false : true;
  if (useIpv6) {
      host = "ff15::1";
  } else {
      host = "224.0.0.1";
  }
  WORD port = 5000;

  auto th = std::thread(server);

  SocketAddr sa(host, port, "udp");
  cout << "Got socket address: " << sa() << endl;
  cout << " port=" << sa.portNumber() << ", protocol=" << sa.protocol()
       << " multicast=" << std::boolalpha << sa.isMulticast() << endl;
  Socket sock;
  if (!sock.open(sa, Socket::modeReadWrite)) {
      cout << "Error opening socket" << endl;
      return 2;
  }
  // Give server a time to start reading, otherwise it will miss the datagrams
  std::this_thread::sleep_for(std::chrono::milliseconds(250));
  sock.write("This is indeed a datagram sent over multicast from sockstr\n");
  cout << "Wrote first message to socket" << endl;
  sock.write("<EOF> This is the last message.\n");
  cout << "Wrote end message to socket, wait for server thread to exit" << endl;
  th.join();
  return 0;
}
