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

#pragma once
//
//   ----------------------------------------------------------------
//   Operations
//
//     NetAddress         Returns the network address in internal format
//     PortNumber         Returns the 16-bit port number
//     operator sockaddr* Returns sockaddr structure which is needed for
//                        calling some of the Winsock routines directly
//     operator LPCSTR    Returns a static, textual representation of an
//                        address (i.e., "host.acme.com:1074")
//
//   ----------------------------------------------------------------
//
// History    : A. Warner, 1996-05-01, Creation
//

#ifdef WINDOWS
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include <string>
#include <variant>

namespace sockstr {

//
// MACRO DEFINITIONS
//
#ifndef DllExport
#define DllExport
#endif

//
// FORWARD CLASS DECLARATIONS
//
class Socket;

/**
 *  This class is used to form a network address that can
 *  be used for sockets.
 */
class DllExport SocketAddr {
public:
    //! Type of special addresses
    enum SpecialIP {
        AddrNone, AddrAny
    };

    //! Type for holding any IP address
    using AddrType = std::variant<std::monostate, sockaddr_in, sockaddr_in6, SpecialIP>;

    /** Construct a SocketAddr object */
    SocketAddr(WORD port, const std::string& protocol = "tcp");
    SocketAddr(const std::string& host, WORD port, const std::string& protocol = "tcp");
    SocketAddr(const std::string& host, const std::string& service, const std::string& protocol = "tcp");

    ~SocketAddr();

    /** Get the sockaddr that is represented by this instance. */
    bool getSockAddr(sockaddr_storage& sa, socklen_t& len);
    /** Return the network address in internal format */
    AddrType netAddress() const;
    /** Return the 16-bit port number for the socket address */
    WORD portNumber() const;
    /**
     * Resolve a given hostname or IP address.
     */
    bool resolve(const std::string& host);
    void setPortNumber(WORD port);

    operator const AddrType () const;
    operator const std::string&();
    const std::string& operator()();

private:
    // Disable copy constructor and assignment operator
    SocketAddr(const SocketAddr&) = delete;
    SocketAddr& operator=(const SocketAddr&) = delete;

    friend class Socket;
    SocketAddr();			// Only called by friend Socket

private:
    //! IP protocol (default is TCP)
    const std::string protocol_;

    //! Storage for resolved address or special (none, any)
    AddrType address_;
        
    //! Peer host name cache
    std::string hostName_;

    WORD portNumber_;

#ifdef WIN32
    //! Used to initialize winsock on (old) Windows
    static unsigned int numInstances_;
#endif
};

}  // namespace sockstr
