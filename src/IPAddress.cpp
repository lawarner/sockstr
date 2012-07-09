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


//
// File       : IPAddress.cpp
//
// Class      : IPAddress
//
// Description: This base class provides mapping between IP addresses
//              and names.  The various names and TCP/IP 'dot addresses'
//              are resolved to a 4 byte network address in internal
//              format.
//
// Decisions  : This class is consistently used by the rest of the IPC
//              library whenever a TCP/IP address is needed.  It is doubtful
//              if an application would ever need to directly use this
//              class.
//
// History    : A. Warner, 1996-05-01, Creation
//

//
// INCLUDE FILES
//
#include "config.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <cassert>
#include <cstdio>
#include <cstring>

#include "sockstr/IPAddress.h"

using namespace sockstr;

//
// MACRO DEFINITIONS
//

//
// TYPE DEFINITIONS
//

//
// FORWARD FUNCTION DECLARATIONS
//

//
// DATA DEFINITIONS
//
// Initialize static member variables

//
// CLASS MEMBER FUNCTION DEFINITIONS
//

// Abstract : Constructs a IPAddress object
//
// Returns  : -
// Params   :
//   lpszName                  Host name or dot address
//   rInAddr                   Reference to existing IP address for copy
//                             constructor
//
// Pre      :
// Post     : All of the constructors first call the initialize() member
//            function that does common initialization (see initialize below).
//            After construction, this object will contain an IP address in
//            the m_dwAddress member variable.
//            A IPAddress object is made and depending on the consructor:
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
//
// Remarks  :
//
IPAddress::IPAddress(void)
:	m_dwAddress(INADDR_ANY)
{
	initialize();
}


IPAddress::IPAddress(const char* lpszName)
:	m_dwAddress(INADDR_ANY)
{
	initialize();

	if (lpszName == 0)
	{
		return;		// must be a server socket
	}

	// First check to see if the name is in valid dot notation
	if ((m_dwAddress = ::inet_addr(lpszName)) != INADDR_NONE)
	{
		return;			// Dot address OK -- use it.
	}

//	struct hostent* pHostEntry = ::gethostbyname(lpszName);
//	if (pHostEntry == 0)
    struct addrinfo aiHints = {
        AI_CANONNAME,
        PF_INET,
        0,
    };
    struct addrinfo* pAddrInfo = 0;
    if (::getaddrinfo(lpszName, 0, &aiHints, &pAddrInfo) || !pAddrInfo)
		return;			// Host name cannot be resolved or nothing returned

    if (pAddrInfo->ai_addr->sa_family == AF_INET)
    {
        m_dwAddress = (in_addr_t)((sockaddr_in *)pAddrInfo->ai_addr)->sin_addr.s_addr;
    }
    ::freeaddrinfo(pAddrInfo);
}


IPAddress::IPAddress(const IPAddress& rInAddr)
{
	initialize();

	m_dwAddress = rInAddr.m_dwAddress;
	strcpy(m_szHostName, rInAddr.m_szHostName);
}


// Abstract : Destructor of IPAddress object
//
// Returns  : -
// Params   :
//   -
//
// Pre      : The Windows sockets library has already been initialized.
// Post     : When the last instance of IPAddress is destroyed, then
//            the Windows sockets library will be shutdown.
//
// Remarks  :
//
IPAddress::~IPAddress(void)
{

}


// Abstract : Perform common initialization for IPAddress object.
//
// Returns  : -
// Params   :
//   -
//
// Pre      : The system where this routine is run must have a Winsock
//            library installed which conforms to the version 1.1 protocol.
// Post     : This routine maintains an instance count of IPAddress
//            objects.  If this is the first instance, then the Windows
//            sockets library is initialized.  This routine also checks
//            that the version of Windows sockets library is compatible
//            with the IPC library.
//
// Remarks  : This routine is only used internally by the IPAddress
//            class.  It does common initializations for all of
//            the constructors.
//
void
IPAddress::initialize(void)
{

}


// Abstract : Returns the internal representation of the IP address.
//
// Returns  : UINT
// Params   :
//   -
//
// Pre      :
// Post     : The IP address is returned in internal, network byte-order format.
//
// Remarks  :
//
UINT
IPAddress::netAddress(void) const
{
	return m_dwAddress;
}


// Abstract : Return a textual representation of IP address
//
// Returns  : char*
// Params   :
//   -
//
// Pre      :
// Post     : A textual representation of the address is returned as either
//            a TCP/IP host name (for example "host.acme.com") or as a
//            dot notation (for example "129.133.133.1").
//
// Remarks  : This routine first attempts to resolve the IP address as a
//            host name.  If this fails, then a string containing the dot
//            notation of the address is returned.  In any case, the result
//            is 'cached' in the m_szHostName member variable (which is the
//            reason this operator is not const).
//
IPAddress::operator char* (void)
{
	struct hostent* pHostEntry;
	
	pHostEntry = ::gethostbyaddr((char *)&m_dwAddress, sizeof(m_dwAddress),
								 AF_INET);
	if (pHostEntry != 0)
	{
		strcpy(m_szHostName, pHostEntry->h_name);
		return m_szHostName;
	}

	// Reverse DNS failed, use TCP/IP dot notation
	sprintf(m_szHostName,"%0d.%0d.%0d.%0d",
		    m_dwAddress & 0xff, (m_dwAddress >> 8) & 0xff,
		   (m_dwAddress >> 16) & 0xff, (m_dwAddress >> 24) & 0xff);
	return m_szHostName;
}

//
// END OF FILE
//
