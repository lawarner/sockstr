/*
   Copyright (C) 2012
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

#ifndef _IPADDRESS_H_INCLUDED_
#define _IPADDRESS_H_INCLUDED_

#include "sstypes.h"

namespace sockstr
{
/**
 * @class IPAddress
 */

//
// MACRO DEFINITIONS
//
#ifndef DllExport
#define DllExport
#endif

/**
 *  The IPAddress class provides mapping between IP addresses and names.
 *  The constructors for this class enable a variety of IP
 *	address types to be created.
 *  The various names and TCP/IP 'dot addresses' are resolved to
 *  a 4 byte network address in internal format.
 */
class DllExport IPAddress
{
public:
    //! Constructs an IPAddress object
    IPAddress(void);
    /**
     * Constructs an IPAddress object.
     *
     * @param  lpszName Host name or dot address
     */
    IPAddress(const char* lpszName);
    /**
	 * Copy constructor for IPAddress object.
     *
     * @param  rInAddr Reference to existing IP address to copy
	 */
	IPAddress(const IPAddress& rInAddr);

	/** Destructs an IPAddress object. */
	~IPAddress(void);

	/**
     * Returns the internal format of network address
     */
	UINT netAddress(void) const;

	/**
     * Returns a static text representation of address
     */
	operator char *(void);

private:
	void initialize(void);

	// Disable assignment operator
	IPAddress& operator=(const IPAddress&);

private:
    // Network address in internal format (network byte order).
    UINT m_dwAddress;
    //struct in_addr m_inAddress;
        
    // Peer host name cache
    char m_szHostName[108];
#ifdef WIN32
    static unsigned int m_uInstances;
#endif
};

}
#endif
