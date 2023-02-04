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

#include <sockstr/sstypes.h>

#include <netinet/in.h>
#include <string>
#include <variant>

namespace sockstr {
/**
 * @class IPAddress
 *
 * The IPAddress class provides mapping between IP addresses and names. The
 * various names, TCP/IP 'dot addresses' or IPv6 addresses are resolved to a
 * sockaddr.
 *
 * This class is consistently used by the rest of the IPC library whenever a
 * TCP/IP address is needed.  It is doubtful if an application would ever
 * need to directly use this class.
 */

#ifndef DllExport
#define DllExport
#endif

class IPAddress {
public:
    //! Type of special addresses
    enum SpecialIP {
        AddrNone, AddrAny
    };
    //! Type for holding any IP address
    using AddrType = std::variant<std::monostate, sockaddr_in, sockaddr_in6, SpecialIP>;

    //! Constructs an IPAddress object with no address
    IPAddress();
    /**
     * Constructs an IPAddress object.
     *
     * @param  lpszName Host name or dot address
     */
    IPAddress(const std::string& host);
    /**
     * Copy constructor for IPAddress object.
     *
     * @param  other Reference to existing IP address to copy
     */
    IPAddress(const IPAddress& other);

    /** Destructs an IPAddress object. */
    ~IPAddress();

    /**
     * Returns the resolved network address (if resolved) or one of the special
     * IP statuses AddrNone or AddrAny.
     */
    AddrType netAddress() const;

    /**
     * Resolve a given hostname or IP address.
     */
    bool resolve(const std::string& host);

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
    operator const std::string&();
    const std::string& operator()();

private:
    /** Perform common initialization for IPAddress object.
     *
     *  On Windows platform, a count of instances is maintained. If
     *  this is the first instance, then the Windows sockets library is
     *  initialized.  This routine also checks that the version of 
     *  Windows sockets library is compatible with the IPC library.
     */
    void initialize();

    // Disable assignment operator
    IPAddress& operator=(const IPAddress&) = delete;

private:
    //! Storage for resolved address or special (none, any)
    AddrType address_;
        
    //! Peer host name cache
    std::string hostName_;

#ifdef WIN32
    //! Used to initialize winsock on (old) Windows
    static unsigned int numInstances_;
#endif
};

}  // namespace sockstr

