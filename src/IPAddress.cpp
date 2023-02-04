/*
   Copyright (C) 2012 - 2022
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

#include "sockstr/IPAddress.h"

#include "config.h"
#ifdef TARGET_WINDOWS
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include <cassert>
#include <cstdio>
#include <cstring>
#include <unistd.h>

namespace sockstr {

//
// Initialize static member variables
#ifdef WIN32
unsigned int IPAddress::numInstances_ = 0;
#endif

// Abstract : Constructs a IPAddress object
//
// Returns  : -
// Params   :
//   lpszName                  Host name, dot address or IPv6 address
//   rInAddr                   Reference to existing IP address for copy
//                             constructor
//
// Pre      :
// Post     : All of the constructors first call the initialize() member
//            function that does common initialization (see initialize below).
//            After construction, this object will contain an IP address in
//            the m_dwAddress member variable.
//            An IPAddress object is made and depending on the consructor:
//            1. an IP address that is not bound to any specific address
//               if the default constructor or IPAddress("") is called.  In
//               this case, the IP address is defined as INADDR_ANY.
//            2. an IP address of 'none' if the lpszName does not resolve to
//               a valid TCP/IP host name.  In this case, the IP address is
//               defined as INADDR_NONE.
//            3. a valid IP address if lpszName is either a valid dot address
//               or host name that resolves correctly.  The IP address in this
//               case is a 4 byte internal representation of the peer's TCP/IP
//               address.
IPAddress::IPAddress()
    : address_(AddrAny) {
    initialize();
}

IPAddress::IPAddress(const std::string& host)
    : address_(AddrNone) {
    initialize();
    if (host.empty()) {	   // must be a server socket
        address_ = AddrAny;
    } else {
        resolve(host);
    }
}


IPAddress::IPAddress(const IPAddress& other) {
    initialize();

    address_ = other.address_;
    hostName_ = other.hostName_;
}


// Pre      : The Windows sockets library has already been initialized.
// Post     : When the last instance of IPAddress is destroyed, then
//
IPAddress::~IPAddress() {
#ifdef WIN32
    VERIFY(numInstances_ > 0);
    // Close the winsock.dll, if no more sockets exist
    if (--numInstances_ == 0) {
        WSACleanup();
    }
#endif
}

void IPAddress::initialize() {
#ifdef WIN32
    // Must initialize Windows Sockets library once before using it.
    if (numInstances_++ == 0) {
        WSADATA wsaData;
        // Initialize winsock.dll
        VERIFY(WSAStartup(MAKEWORD(2,2), &wsaData) == 0);

        // Confirm that the Windows Sockets DLL supports 2.2
        if (LOBYTE(wsaData.wVersion) != 2 ||
            HIBYTE(wsaData.wVersion) != 2) {
            // Wrong version of winsock -- cannot be used
            WSACleanup();
            VERIFY(0);
        }
    }
#endif
}

IPAddress::AddrType IPAddress::netAddress() const {
    return address_;
}

bool IPAddress::resolve(const std::string& host) {
    constexpr const char* validIpv4 = "0123456789.";
    constexpr const char* validIpv6 = "0123456789abcdefABCDEF:";
    bool is_valid = false;

    if (host.find_first_not_of(validIpv4) == std::string::npos) {
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
        int ret = getaddrinfo(host.c_str(), nullptr, &hints, &addr);
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
                    address_ = *(sockaddr_in *)(next->ai_addr);
                } else {
                    address_ = *(sockaddr_in6 *)(next->ai_addr);
                }
            }
            freeaddrinfo(addr);
        }            
    }
    return is_valid;
#if 0
    // First check to see if the name is in valid dot notation
    if ((m_dwAddress = ::inet_addr(lpszName)) != INADDR_NONE) {
        return;			// Dot address OK -- use it.
    }

//	struct hostent* pHostEntry = ::gethostbyname(lpszName);
//	if (pHostEntry == 0)

    struct addrinfo aiHints = {
        .ai_flags = AI_CANONNAME,
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = IPPROTO_TCP
        // ai_addrlen
        // ai_canonname (char*)
        // ai_next
    };
    struct in_addr inad;
    struct in6_addr in6ad;
    if (::inet_pton(AF_INET, lpszName, &inad)) {
        // IPv4 address
        aiHints.ai_flags  = AI_NUMERICSERV | AI_NUMERICHOST;
        aiHints.ai_family = AF_INET;
    } else if (::inet_pton(AF_INET6, lpszName, &in6ad)) {
        // IPV6 address
        aiHints.ai_flags  = AI_NUMERICSERV | AI_NUMERICHOST;
        aiHints.ai_family = AF_INET6;
    } else {
        // Try a hostname
        struct hostent* he = ::gethostbyname(lpszName);
        if (he == nullptr) {
            m_dwAddress = INADDR_NONE;
            return;			// Host name cannot be resolved or nothing returned
        } else {
            in_addr_t **addr_list = (in_addr_t **)he->h_addr_list;
            m_dwAddress = *addr_list[0];  // Just grab the first one
        }
        return;
    }

    struct addrinfo* pAddrInfo = nullptr;
    if (::getaddrinfo(lpszName, 0, &aiHints, &pAddrInfo) || !pAddrInfo) {
        m_dwAddress = INADDR_NONE;
        return;			// Host name cannot be resolved or nothing returned
    }
    if (pAddrInfo->ai_addr->sa_family == AF_INET ||
        pAddrInfo->ai_addr->sa_family == AF_INET6)
    {
#ifdef TARGET_LINUX
        m_dwAddress = (in_addr_t)((sockaddr_in *)pAddrInfo->ai_addr)->sin_addr.s_addr;
#else
        m_dwAddress = ((sockaddr_in *)pAddrInfo->ai_addr)->sin_addr.s_addr;
#endif
    }
    ::freeaddrinfo(pAddrInfo);
    return true;
#endif
}

IPAddress::operator const std::string& () {
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
    }
#if 0
    struct hostent* pHostEntry;
    pHostEntry = ::gethostbyaddr((char *)&m_dwAddress, sizeof(m_dwAddress),
                                 AF_INET);
    if (pHostEntry != nullptr) {
        strcpy(m_szHostName, pHostEntry->h_name);
        return m_szHostName;
    }

    // Reverse DNS failed, use TCP/IP dot notation
    sprintf(m_szHostName,"%0d.%0d.%0d.%0d",
            m_dwAddress & 0xff, (m_dwAddress >> 8) & 0xff,
            (m_dwAddress >> 16) & 0xff, (m_dwAddress >> 24) & 0xff);
#endif
    return hostName_;
}

const std::string& IPAddress::operator()() {
    return operator const std::string&();
}

}  // namespace sockstr
