/*
   Copyright (C) 2012 - 2026
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
//     AddrType           Returns the network address in internal format
//     portNumber         Returns the 16-bit port number
//     protocol           Returns the name of protocol (tcp or udp)
//     operator std::string&  Returns a static, textual representation of
//                            an address (i.e., "host.acme.com:1074")
//
//   ----------------------------------------------------------------
//
// History    : A. Warner, 1996-05-01, Creation
//

#include <sockstr/sstypes.h>

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
 *  be used for sockets. It can be constructed by providing a
 *  host name, TCP/IP 'dot address' or IPv6 address together
 *  with a port number.
 */
class DllExport SocketAddr {
public:
    //! Type of special addresses
    enum SpecialIP {
        AddrNone, AddrAny
    };

    //! Type for holding any IP address
    using AddrType = std::variant<std::monostate, sockaddr_in, sockaddr_in6, SpecialIP>;

    /**
     * Construct a SocketAddr object.
     * Since no host name is supplied, the SocketAddr is to be used for server-side sockets.
     *
     * @param port  16-bit port number
     * @param protocol Protocol such as "udp". Defaults to "tcp" if not specified.
     */
    SocketAddr(WORD port, const std::string& protocol = "tcp");
    /**
     * Construct a SocketAddr object.
     * Since no host name is supplied, the SocketAddr is to be used for server-side sockets.
     *
     * @param host  Host name, dot address or IPv6 address
     * @param port  16-bit port number
     * @param protocol Protocol such as "udp". Defaults to "tcp" if not specified.
     */
    SocketAddr(const std::string& host, WORD port, const std::string& protocol = "tcp");
    /**
     * Construct a SocketAddr object.
     * Since no host name is supplied, the SocketAddr is to be used for server-side sockets.
     *
     * @param host  Host name, dot address or IPv6 address.
     * @param service  A known service name (such as "ntp") that is combined with the
     *                  protocol to form a 16-bit port number.
     * @param protocol Protocol such as "udp". Defaults to "tcp" if not specified.
     */
    SocketAddr(const std::string& host, const std::string& service, const std::string& protocol = "tcp");

    /** Destructs a SocketAddr */
    ~SocketAddr();

    /** Get the sockaddr that is represented by this instance.
     *  @param sa   Provided storage where the sockaddr is copied into
     *  @param len  Length of data returned
     *  @return True if a valid sockaddr was returned, otherwise false.
     */
    bool getSockAddr(sockaddr_storage& sa, socklen_t& len);
    /** Return the host name as a string if the address can be resolved as a name.
     *  If name resolution fails then returns a textual representation of the IP
     *  address in either IPv4 or IPv6 form.
     */
    std::string hostname();
    /** Return whether or not this socket address is to be used for multicast.
     *  @return True if this is a multicast address, otherwise false.
     */
    bool isMulticast() const;
    /** Return the network address (if resolved) in internal format
     *  or one of the special IP statuses AddrNone or AddrAny.
     */
    AddrType netAddress() const;
    /** Return the 16-bit port number for the socket address */
    WORD portNumber() const;
    /** Return the protocol */
    const std::string& protocol() const;
    /**
     * Resolve a given hostname or IP address.
     *
     * @param host  Host name, dot address or IPv6 address.
     * @return True if the given host can be resovled, otherwise false.
     */
    bool resolve(const std::string& host);
    void setPortNumber(WORD port);

    operator const AddrType () const;

    /**
     * Returns a static text representation of the resolved host's address.
     * This can be either a TCP/IP host name (for example "host.acme.com")
     * or as a IPv4 dot notation (for example "129.133.133.1") or a IPv6
     * address such as ::1.
     *
     * Note this routine first attempts to resolve the IP address as a host
     * name.  If this fails, then a string containing the dot notation or
     * colonized IPv6 of the address is returned.  In any case, the result
     * is 'cached' inside this class.
     */
    operator std::string();
    const std::string operator()();

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
    bool isMulticast_;
#ifdef WIN32
    //! Used to initialize winsock on (old) Windows
    static unsigned int numInstances_;
#endif
};

}  // namespace sockstr
