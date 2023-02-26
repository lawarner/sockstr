/*
   Copyright (C) 2012 - 2023
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


// Description: This class is used as a wrapper around the
//              sockets structure, sockaddr_in.  It provides an easy and
//              consist manner of constructing IP addresses and port number
//              pairs that are needed in subsequent socket calls.
//              Various forms of construction provide a large degree of
//              flexibility (see constructors below).
//
// Decisions  : This class holds a TCP/IP name  and an optional port number or service name.
//              This class is sometimes useful for an application program
//              to use, for example when a Socket is constructed:
//                      Socket(SocketAddr("host", 1701), ...);
//
// History    : A. Warner, 1996-05-01, Creation
//

#include "config.h"
#include <sockstr/SocketAddr.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#ifdef TARGET_LINUX
#include <netdb.h>
#endif

using namespace sockstr;

// Params   :
//   Host                      An IP address object
//   wPort                     TCP/IP port number
//   lpszHost                  Host name of peer system (for client sockets)
//   service               Service name for TCP/IP port
//
// Pre      :
// Post     : A SocketAddr object is made and depending on which
//            constructor is called and how it is called a particular
//            socket address is created.  During the construction, the
//            member variables inherited from sockaddr_in are filled-in.
//            The following describes how each of the constructors work:
//            1. The default constructor creates an address that can only be
//               used for a server-side socket.  The IP address is left open
//               allowing a connection from any client.  The port number will
//               be generated by the user of this class.
//            2. The other constructor create either a client socket or a server
//               socket depending on how the parameters are filled-in.  When
//               the Host parameter is used (second constuctor) then the type
//               of socket address is the same as the Host parameter.
//               When the lpszHost parameter is empty, then a server socket is
//               created.  If lpszHost resolves to a valid TCP/IP address then
//               a client socket address is created.  Otherwise, the address
//               is set to 'none', which means the socket cannot be used.
//            The internal network address is set.
//
// Remarks  : The default constructor is private and is used only by friend
//            class Socket.
//
SocketAddr::SocketAddr()
    : protocol_()
    , address_(AddrAny)
    , portNumber_(0) {}

SocketAddr::SocketAddr(WORD port, const std::string& protocol)
    : protocol_(protocol)
    , address_(AddrNone)
    , portNumber_(port) {
    resolve(std::string());
}

SocketAddr::SocketAddr(const std::string& host, WORD port, const std::string& protocol)
    : protocol_(protocol)
    , address_(AddrNone)
    , portNumber_(port) {
    resolve(host);
}

SocketAddr::SocketAddr(const std::string& host, const std::string& service, const std::string& protocol)
    : protocol_(protocol)
    , address_(AddrNone)
    , portNumber_(0) {

    struct servent* pService = ::getservbyname(service.c_str(), protocol.c_str());
    if (pService &&
#ifdef TARGET_WINDOWS
        _stricmp(pService->s_proto, protocol.empty() ? "tcp" : protocol.c_str()) == 0)
#else
        strcasecmp(pService->s_proto, protocol.empty() ? "tcp" : protocol.c_str()) == 0)
#endif
    {
        portNumber_ = pService->s_port;
    }
    resolve(host);
}

SocketAddr::~SocketAddr() {}

bool SocketAddr::getSockAddr(sockaddr_storage& sa, socklen_t& len) {
    struct FillSaStorage {
        FillSaStorage(sockaddr_storage* sas, WORD port)
            : sa_store_(sas)
            , port_(port) {}
        socklen_t operator()(std::monostate& ) {
            return 0;
        }
        socklen_t operator()(sockaddr_in& sin) {
            memcpy(sa_store_, &sin, sizeof(sockaddr_in));
            return sizeof(sockaddr_in);
        }
        socklen_t operator()(sockaddr_in6& sin6) {
            memcpy(sa_store_, &sin6, sizeof(sockaddr_in6));
            return sizeof(sockaddr_in6);
        }
        socklen_t operator()(SpecialIP& sip) {
            // TODO does not express INADDR_NONE (is it still needed?)
            //auto spec = std::get<SpecialIP>(address_);
            sockaddr_in6 si = {
                .sin6_family = AF_INET6,
                .sin6_port = htons(port_),
                .sin6_addr = in6addr_any
            };
            memcpy(sa_store_, &si, sizeof(si));
            return sizeof(si);
        }

        sockaddr_storage* sa_store_;
        WORD port_;
    };
    FillSaStorage filler(&sa, portNumber_);
    len = std::visit(filler, address_);
    return len > 0;
}

SocketAddr::AddrType SocketAddr::netAddress() const {
    return address_;
}

WORD SocketAddr::portNumber() const {
    return portNumber_;
}

bool SocketAddr::resolve(const std::string& host) {
    constexpr const char* validIpv4 = "0123456789.";
    constexpr const char* validIpv6 = "0123456789abcdefABCDEF:";
    bool is_valid = false;

    // TODO use portNumber_
    if (host.empty()) {
        address_ = AddrAny;
    } else if (host.find_first_not_of(validIpv4) == std::string::npos) {
        // Looks like dot notation, so try IPv4 address
        struct sockaddr_in sa;
        memset(&sa, 0, sizeof(struct sockaddr_in));
        sa.sin_family = AF_INET;
        auto ret = inet_pton(AF_INET, host.c_str(), &sa.sin_addr);
        if (ret > 0) {
            address_ = sa;
            is_valid = true;
        } else {
            address_ = AddrNone;
        }
    } else if (host.find_first_not_of(validIpv6)) {
        // Try IPv6 address
        sockaddr_in6 sa6;
        sa6.sin6_family = AF_INET6;
        auto ret = inet_pton(AF_INET6, host.c_str(), &sa6.sin6_addr);
        if (ret > 0) {
            address_ = sa6;
            is_valid = true;
        } else {
            address_ = AddrNone;
        }
    } else {     // Try to resolve host name
        struct addrinfo* addr;
        struct addrinfo hints = {
            .ai_flags = 0,
            .ai_family = AF_UNSPEC,
            .ai_socktype = SOCK_STREAM,
            .ai_protocol = IPPROTO_TCP
        };
        std::string portNum = portNumber_ == 0 ? "" : std::to_string(portNumber_);
        int ret = getaddrinfo(host.c_str(), portNum.c_str(), &hints, &addr);
        if (!ret) {
            struct addrinfo* next;
            for (next = addr; next != nullptr; next = next->ai_next) {
                int s = socket(next->ai_family, next->ai_socktype, next->ai_protocol);
                if (s >= 0) {
                    close(s);
                    is_valid = true;
                    break;
                }
            }
            if (next == nullptr) {
                address_ = AddrNone;
            } else {
                if (next->ai_family == AF_INET) {
                    auto sa = (sockaddr_in *)(next->ai_addr);
                    sa->sin_port = htons(portNumber_);
                    address_ = *sa;
                } else {
                    auto sa6 = (sockaddr_in6 *)(next->ai_addr);
                    sa6->sin6_port = htons(portNumber_);
                    address_ = *sa6;
                }
            }
            freeaddrinfo(addr);
        }            
    }
    return is_valid;
}

void SocketAddr::setPortNumber(WORD port) {
    portNumber_ = port;
}
    
// Abstract : Returns a pointer to the network address in internal format.

SocketAddr::operator const AddrType () const {
    return address_;
}

// Abstract : Returns a static, textual representation of an address
//
// Pre      :
// Post     : Returns a text string representing the socket address.  Note
//            that no attempt is made to preserve the contents of this
//            string between subsequent calls.  The application should not
//            attempt to modify or free the string returned.
//            The string returned is a concatenation of the TCP/IP address
//            (host name or dot notation) and the port number.  For example,
//            "hostb.omroep.nl:7".
//
SocketAddr::operator const std::string& () {
    if (hostName_.empty()) {
        char hbuf[NI_MAXHOST];
        if (std::holds_alternative<sockaddr_in>(address_)) {
            auto sa = (sockaddr *) &std::get<sockaddr_in>(address_);
            if (!getnameinfo(sa, sizeof(sockaddr_in), hbuf, sizeof(hbuf),
                             nullptr, 0, NI_NAMEREQD)) {
                hostName_ = hbuf;
            }
        } else if (std::holds_alternative<sockaddr_in6>(address_)) {
            auto sa6 = (sockaddr *) &std::get<sockaddr_in6>(address_);
            if (!getnameinfo(sa6, sizeof(sockaddr_in), hbuf, sizeof(hbuf),
                             nullptr, 0, NI_NAMEREQD)) {
                hostName_ = hbuf;
            }
        }
        if (portNumber()) {
            hostName_.append(":" + std::to_string(portNumber()));
        }
    }
    return hostName_;
}

const std::string& SocketAddr::operator()() {
    return operator const std::string&();
}
