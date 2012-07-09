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

#ifndef _SOCKETADDR_H_INCLUDED_
#define _SOCKETADDR_H_INCLUDED_
//
// File       : SocketAddr.h
//
// Class      : SocketAddr
//
// Hierarchy  : sockaddr_in
//               |
//               +-- SocketAddr
//
// Description: This class is used to form a network address that can
//              be used for sockets.
//
// Members    :
//   Data
//
//   ----------------------------------------------------------------
//   Construction
//
//     SocketAddr Constructs a SocketAddr object
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
//   Overridables
//
//     -
//
//
// History    : A. Warner, 1996-05-01, Creation
//


//
// INCLUDE FILES
//
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sockstr/IPAddress.h>

namespace sockstr
{

//
// MACRO DEFINITIONS
//
#ifndef DllExport
#define DllExport
#endif

//
// TYPE DEFINITIONS
//

//
// FORWARD CLASS DECLARATIONS
//
class Socket;

//
// DATA DECLARATIONS
//

//
// FUNCTION DECLARATIONS
//

//
// CLASS DECLARATION
class DllExport SocketAddr : public sockaddr_in
{
public:
	SocketAddr(const IPAddress Host, WORD wPort, const char * pProtocol = 0);
	SocketAddr(const char * lpszHost, WORD wPort, const char * pProtocol = 0);
	SocketAddr(const char * lpszHost, const char * lpszService, const char * pProtocol = 0);

	~SocketAddr(void);

	UINT netAddress(void) const;
	WORD portNumber(void) const;

	operator sockaddr* (void) const;
	operator LPCSTR    (void) const;

private:
	// Disable copy constructor and assignment operator
	SocketAddr(const SocketAddr&);
	SocketAddr& operator=(const SocketAddr&);

	friend class Socket;
	SocketAddr(void);			// Only called by friend Socket

private:
	IPAddress* m_pPeerAddr;		// Peer host name cache
	const char*    m_pProtocol;		// IP protocol (default is TCP)
};

}
#endif
