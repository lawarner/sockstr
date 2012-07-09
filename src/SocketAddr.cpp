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
// File       : SocketAddr.cpp
//
// Class      : SocketAddr
//
// Description: This class is used as a wrapper around the standard Windows
//              sockets structure, sockaddr_in.  It provides an easy and
//              consist manner of constructing IP addresses and port number
//              pairs that are needed in subsequent Windows sockets calls.
//              Various forms of construction provide a large degree of
//              flexibility (see constructors below).
//
// Decisions  : This class is closely related to the IPAddress class.
//              The primary difference being that this class combines the
//              TCP/IP name with a port number or service name.
//              This class is sometimes useful for an application program
//              to use, for example when a Socket is constructed:
//                      Socket(SocketAddr("host", 1701), ...);
//
//
// History    : A. Warner, 1996-05-01, Creation
//

//
// INCLUDE FILES
//
#include "config.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <sockstr/SocketAddr.h>

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

//
// CLASS MEMBER FUNCTION DEFINITIONS
//

// Abstract : Constructs a SocketAddr object
//
// Returns  : -
// Params   :
//   Host                      An IP address object
//   wPort                     TCP/IP port number
//   lpszHost                  Host name of peer system (for client sockets)
//   lpszService               Service name for TCP/IP port
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
SocketAddr::SocketAddr(void)
:	m_pProtocol (0)
{
	m_pPeerAddr     = new IPAddress;
	sin_family      = AF_INET;
	sin_port        = 0;
	sin_addr.s_addr = m_pPeerAddr->netAddress();
}


SocketAddr::SocketAddr(const IPAddress Host, WORD wPort,
							   const char * pProtocol /*= NULL*/)
:	m_pProtocol (pProtocol)
{
	// Call the copy constructor
	m_pPeerAddr     = new IPAddress(Host);
	sin_family      = AF_INET;
	sin_port        = htons(wPort);
	sin_addr.s_addr = m_pPeerAddr->netAddress();
}


SocketAddr::SocketAddr(const char * lpszHost, WORD wPort,
							   const char * pProtocol /*= 0*/)
:	m_pProtocol (pProtocol)
{
	m_pPeerAddr     = new IPAddress(lpszHost);
	sin_family      = AF_INET;
	sin_port        = htons(wPort);
	sin_addr.s_addr = m_pPeerAddr->netAddress();
}


SocketAddr::SocketAddr(const char * lpszHost, const char * lpszService,
							   const char * pProtocol /*= 0*/)
:	m_pProtocol (pProtocol)
{
	// WARNING:  The following statement MUST precede the call to
	//           getservbyname.  IPAddress inits Windows sockets.
	m_pPeerAddr = new IPAddress(lpszHost);

	struct servent* pService = ::getservbyname(lpszService, pProtocol);
	sin_family      = AF_INET;
	sin_addr.s_addr = m_pPeerAddr->netAddress();

	if (pService &&
		strcasecmp(pService->s_proto, pProtocol == 0 ? "tcp" : pProtocol) == 0)
	{
		sin_port = pService->s_port;
	}
	else
	{
		sin_port = 0;
	}
}


// Abstract : Destructor of SocketAddr object
//
// Returns  : -
// Params   :
//   -
//
// Pre      :
// Post     : The SocketAddr object is destructed
//
// Remarks  :
//
SocketAddr::~SocketAddr(void)
{
	delete m_pPeerAddr;
}


// Abstract : Returns the network address in internal format
//
// Returns  : UINT
// Params   :
//   -
//
// Pre      :
// Post     : The internal network address is returned
//
// Remarks  :
//
UINT
SocketAddr::netAddress(void) const
{
	return m_pPeerAddr->netAddress();
}


// Abstract : Returns the port number for the socket address
//
// Returns  : WORD
// Params   :
//   -
//
// Pre      :
// Post     : The 16-bit port number for the current object is returned
//
// Remarks  :
//
WORD
SocketAddr::portNumber(void) const
{
	return ntohs(sin_port);
}


// Abstract : Returns a pointer to the sockaddr structure which is
//            needed for calling some of the Winsock routines directly
//
// Returns  : sockaddr*
// Params   :
//   -
//
// Pre      :
// Post     : A pointer to the sockaddr structure is returned.
//
// Remarks  :
//
SocketAddr::operator sockaddr* (void) const
{
	return (sockaddr *)this;
}


// Abstract : Returns a static, textual representation of an address
//
// Returns  : LPCSTR
// Params   :
//   -
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
// Remarks  :
//
SocketAddr::operator char * (void) const
{
	static char szFullName[100];
	sprintf(szFullName, "%s:%d", (char *) *m_pPeerAddr, portNumber());
	return  szFullName;
}

//
// END OF FILE
//
