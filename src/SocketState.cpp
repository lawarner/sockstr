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
// File       : SocketState.cpp
//
// Class      : SocketState
//
// Description: The SocketState class and its sub-classes are actually
//              a part of the Socket class.  These classes implement
//              the state-variant part of Socket's interface.
//              SocketState and its sub-classes thus form a state tree.
//
// Decisions  : Note that the use of SocketState is a design decision.
//              It is used to distribute state-dependent behavior across
//              several subclasses.  Therefore, the functionality of the
//              Socket class is distributed over many subclasses.
//              A possible drawback is that changes to a state tree could
//              result in several classes needing to be changed.  On the
//              other hand, such distribution is actually good if there are
//              many states.  The code for each state is isolated from the
//              other states and is thus easier to understand.  If the state
//              tree method is not used, then some other approach is needed,
//              which often results in a large switch statement, or some
//              other monolithic programming construction.
//              The state transitions are performed by the subclasses
//              themself.  In other words, a state subclass has knowlege of
//              which state follows.  However, this is usually not a problem.
//              Instances of a state subclass can be shared unless they
//              define member variables.  The static member function
//              instance() is used when sharing is enabled.
//

//
// INCLUDE FILES
//
#include "config.h"
#include <cassert>
#ifdef TARGET_LINUX
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
#ifdef TARGET_WINDOWS
#include <WinSock2.h>
#include <WS2tcpip.h>
#endif

#include <algorithm>
#include <iostream>

#include <sockstr/Socket.h>
#include <sockstr/SocketState.h>

#if USE_OPENSSL
#include <openssl/ssl.h>
#endif

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

// Initialize static members
SocketState* SSClosed::m_pInstance = 0;
SocketState* SSOpenedServer::m_pInstance = 0;
SocketState* SSOpenedClient::m_pInstance = 0;
SocketState* SSListening::m_pInstance = 0;
SocketState* SSConnected::m_pInstance = 0;
#if USE_OPENSSL
SocketState* SSOpenedClientTLS::m_pInstance = 0;
SocketState* SSConnectedTLS::m_pInstance = 0;
#endif

//
// CLASS MEMBER FUNCTION DEFINITIONS
//

// Abstract : Constructs a SocketState
//
// Returns  : -
// Params   :
//   -
//
// Pre      :
// Post     :
//
// Remarks  :
//
SocketState::SocketState(void)
{

}


// Abstract : Destructor of SocketState object
//
// Returns  : -
// Params   :
//   -
//
// Pre      :
// Post     : The SocketState object is destructed.
//
// Remarks  :
//
SocketState::~SocketState(void)
{

}


// State-dependent functions

// Abstract : Cancel any outstanding blocking socket events.
//
// Returns  : -
// Params   :
//   pSocket                   Pointer to socket object
//
// Pre      :
// Post     : Pending I/O on the socket is cancelled.
//
// Remarks  : <only when required, not obligatory>
//
void
SocketState::abort(Socket* pSocket)
{
	// Not interested in return value, just always try to abort blocking I/O
	if (pSocket->m_hFile)
	{
#ifdef TARGET_WINDOWS
		::WSACancelBlockingCall();
#endif
	}
}


// Abstract : Closes the socket connection
//
// Returns  : -
// Params   :
//   pSocket                   Pointer to socket object
//
// Pre      :
// Post     : The socket connection is closed if it was open.  The pSocket
//            object is switched to a closed state.
//
// Remarks  :
//
void
SocketState::close(Socket* pSocket)
{
    // Flush any characters remaining in the streambuf
    pSocket->strbuf.pubsync();

	// Not interested in return value, just always try to close socket
#ifdef TARGET_WINDOWS
	::closesocket(pSocket->m_hFile);
#else
	::close(pSocket->m_hFile);
#endif
    pSocket->m_hFile = INVALID_SOCKET;

	changeState(pSocket, SSClosed::instance());
}


// Abstract : Interface abstraction to create a server-side socket
//
// Returns  : Reference to a new Socket object for the server
// Params   :
//   pSocket                   Pointer to socket object
//   wPort                     TCP/IP port number of the new socket
//   pHost                     IP address of already open client-side connection
//
// Pre      : The pHost parameter must already be resolved to a valid TCP/IP host.
//            The pSocket parameter must point to an already open socket connection.
// Post     :
//
// Remarks  : This routine is only a "place-holder" for the abstract
//            interface.  Sub-classes of SocketState that can legally
//            perform this action will override this function and
//            provide the state specific implementation code.
//            All other state sub-classes (that can't legally perform this
//            action) will be trapped by this function whose implementation
//            is to abort the program, signifying that an illegal action
//            was attempted.
//
//            createServer IS NOT YET IMPLEMENTED !!!
//
Socket&
SocketState::createServer(Socket* /*pSocket*/,
						 const WORD  /*wPort*/,
						 const IPAddress* /*pHost*/)
{
	VERIFY(0);	// We should never execute this base class virtual function
	return *new Socket;
}


// Abstract : Wrapper function for the standard socket getsockopt routine
//
// Returns  : true if the call was successful, otherwise false
// Params   :
//   pSocket                   Pointer to socket object
//   nOptionName               Index of the socket option (i.e., SO_KEEPALIVE)
//   pOptionValue              Pointer to buffer where option will be copied
//   pnOptionLen               Length of pOptionValue buffer
//   nLevel                    Level number of the stack where the pOptionName
//                             option resides
//
// Pre      :
// Post     : The value of the requested option is retrieved into the user's
//            buffer.  The actual length that was copied is returned in the
//            nOptionLen parameter.
//
// Remarks  : This routine is only a "place-holder" for the abstract
//            interface.  Sub-classes of SocketState that can legally
//            perform this action will override this function and
//            provide the state specific implementation code.
//            All other state sub-classes (that can't legally perform this
//            action) will be trapped by this function whose implementation
//            is to abort the program, signifying that an illegal action
//            was attempted.
//
bool
SocketState::getSockOpt(Socket* pSocket,
                        int nOptionName, void* pOptionValue,
                        socklen_t* pnOptionLen,  int nLevel)
{
    if (pSocket->getHandle() == INVALID_SOCKET)
        return false;

	if (::getsockopt(pSocket->getHandle(), nLevel, nOptionName, (char *)pOptionValue,
					 pnOptionLen) == SOCKET_ERROR)
	{
		return false;
	}
	return true;
}


// Abstract : Listen on a server-side socket for incoming connection requests
//
// Returns  : Socket handle of incoming connection
// Params   :
//   pSocket                   Pointer to socket object
//   nBackLog                  Maximum number of clients that can be
//                             queued waiting for a connection
//
// Pre      :
// Post     : Upon successful completion of this routine, a new socket handle
//            is created which is connected to a client.  The original server
//            socket remains in listen mode.
//
// Remarks  : This routine is only a "place-holder" for the abstract
//            interface.  Sub-classes of SocketState that can legally
//            perform this action will override this function and
//            provide the state specific implementation code.
//            All other state sub-classes (that can't legally perform this
//            action) will be trapped by this function whose implementation
//            is to abort the program, signifying that an illegal action
//            was attempted.
//
SOCKET
SocketState::listen(Socket* /*pSocket*/, const int /*nBacklog*/)
{
	VERIFY(0);	// We should never execute this base class virtual function
	return false;
}


// Abstract : Open a socket as specified by a SocketAddr address
//
// Returns  : true on success
// Params   :
//   pSocket                   Pointer to socket object
//   rSockAddr                 Reference to a socket address to open
//   uOpenFlags                Flags indicating how the socket should be opened
//
// Pre      : This routine expects a well-formed SocketAddr.
// Post     : Upon successful completion of this routine, either a server
//            socket or a client socket will be opened.  In the case of
//            a server socket, the application will need to call Listen in
//            order to connect the socket to a client.
//
// Remarks  : This routine is only a "place-holder" for the abstract
//            interface.  Sub-classes of SocketState that can legally
//            perform this action will override this function and
//            provide the state specific implementation code.
//            All other state sub-classes (that can't legally perform this
//            action) will be trapped by this function whose implementation
//            is to abort the program, signifying that an illegal action
//            was attempted.
//
bool
SocketState::open(Socket* /*pSocket*/,
                  SocketAddr& /*rSockAddr*/,
                  UINT /*uOpenFlags*/)
{
	VERIFY(0);	// We should never execute this base class virtual function
	return false;
}


// Abstract : Read in a specified number of raw bytes from socket
//
// Returns  : UINT (actual number of bytes read)
// Params   :
//   pSocket                   Pointer to socket object
//   pBuf                      Buffer that will be filled
//   uCount                    Size of buffer
//
// Pre      :
// Post     : This routine will fill pBuf with data that is available
//            from the socket.  It will read no more than the maximum
//            given by the uCount parameter.  This routine returns the
//            number of bytes actually read.
//
// Remarks  : This routine is only a "place-holder" for the abstract
//            interface.  Sub-classes of SocketState that can legally
//            perform this action will override this function and
//            provide the state specific implementation code.
//            All other state sub-classes (that can't legally perform this
//            action) will be trapped by this function whose implementation
//            is to abort the program, signifying that an illegal action
//            was attempted.
//
UINT
SocketState::read(Socket* /*pSocket*/,
					  void* /*pBuf*/, UINT /*uCount*/)
{
	VERIFY(0);	// We should never execute this base class virtual function
	return 0;
}


// Abstract : Process for the read worker thread
//
// Returns  : 0 if successful, 1 if an error occurred
// Params   :
//   pIOP                      pointer to the I/O parameters
//
// Pre      : The socket connection has already been established and the
//            Socket object is set to asynchronous I/O mode.
// Post     :
//
// Remarks  : This routine is only a "place-holder" for the abstract
//            interface.  Sub-classes of SocketState that can legally
//            perform this action will override this function and
//            provide the state specific implementation code.
//            All other state sub-classes (that can't legally perform this
//            action) will be trapped by this function whose implementation
//            is to abort the program, signifying that an illegal action
//            was attempted.
//
DWORD
SocketState::readerThread(IOPARAMS* /*pIOP*/)
{
	VERIFY(0);	// We should never execute this base class virtual function
	return 0;
}


// Abstract : Wrapper function for the standard socket setsockopt routine
//
// Returns  : true if the call was successful, otherwise false
// Params   :
//   pSocket                   Pointer to socket object
//   nOptionName               Index of the socket option (i.e., SO_KEEPALIVE)
//   pOptionValue              Pointer to buffer which contains the option value
//   nOptionLen                Length of pOptionValue buffer
//   nLevel                    Level number of the stack where the pOptionName
//                             option resides
//
// Pre      :
// Post     : The specified option is set to the value from the user's buffer.
//
// Remarks  : This routine is only a "place-holder" for the abstract
//            interface.  Sub-classes of SocketState that can legally
//            perform this action will override this function and
//            provide the state specific implementation code.
//            All other state sub-classes (that can't legally perform this
//            action) will be trapped by this function whose implementation
//            is to abort the program, signifying that an illegal action
//            was attempted.
//
bool
SocketState::setSockOpt(Socket* pSocket,
                         int nOptionName, const void* pOptionValue,
                         int nOptionLen, int nLevel)
{
    if (pSocket->getHandle() == INVALID_SOCKET)
        return false;

	if (::setsockopt(pSocket->getHandle(), nLevel, nOptionName, (char *)pOptionValue,
					 nOptionLen) == SOCKET_ERROR)
	{
		return false;
	}
	return true;
}


// Abstract : Write the specified number of raw bytes from the user's buffer
//            to the socket.
//
// Returns  : -
// Params   :
//   pBuf                      Buffer that will be written
//   uCount                    Number of bytes to write
//
// Pre      :
// Post     : This routine will write uCount bytes of data from pBuf to
//            the socket.
//
// Remarks  : This routine is only a "place-holder" for the abstract
//            interface.  Sub-classes of SocketState that can legally
//            perform this action will override this function and
//            provide the state specific implementation code.
//            All other state sub-classes (that can't legally perform this
//            action) will be trapped by this function whose implementation
//            is to abort the program, signifying that an illegal action
//            was attempted.
//
void
SocketState::write(Socket* /*pSocket*/,
					   const void* /*pBuf*/, UINT /*uCount*/)
{
	VERIFY(0);	// We should never execute this base class virtual function
}


// Abstract : Process for the write worker thread
//
// Returns  : 0 if successful, 1 if an error occurred
// Params   :
//   pIOP                      pointer to the I/O parameters
//
// Pre      : The socket connection has already been established and the
//            Socket object is set to asynchronous I/O mode.
// Post     :
//
// Remarks  : This routine is only a "place-holder" for the abstract
//            interface.  Sub-classes of SocketState that can legally
//            perform this action will override this function and
//            provide the state specific implementation code.
//            All other state sub-classes (that can't legally perform this
//            action) will be trapped by this function whose implementation
//            is to abort the program, signifying that an illegal action
//            was attempted.
//
DWORD
SocketState::writerThread(IOPARAMS* /*pIOP*/)
{
	VERIFY(0);	// We should never execute this base class virtual function
	return 0;
}


// Abstract : Goes to the next specified state
//
// Returns  : -
// Params   :
//   pSocket                   Pointer to the Socket object
//   pState                    Pointer specifying the next state
//
// Pre      :
// Post     : The socket object pSocket is switched to the state pState
//
// Remarks  :
//
void
SocketState::changeState(Socket* pSocket, SocketState* pState)
{
	pSocket->changeState(pState);
}


// Abstract : Creates and fills an IOPARAMS structure
//
// Returns  : A pointer to a newly allocated IOPARAMS structure that is filled
//            in with the calling parameters to this routine.
// Params   :
//   pSocket                   Pointer to socket object
//   pBuf                      Buffer that will be read or written
//   uCount                    Number of bytes (size of buffer)
//   pCallback                 Pointer to the user's callback routine
//
// Pre      :
// Post     : A new IOPARAMS structure is allocated and its members, m_pSocket,
//            m_pBuf, m_uCount, and m_pCallback are filled with parameters to
//            this function.
//
// Remarks  : Only used by SocketState and its sub-classes.  It is the
//            responsibility of the caller to free the returned buffer when
//            it is no longer needed.
//            The reason this routine is needed is to pass several parameters
//            to the worker threads.
//
IOPARAMS*
SocketState::createIOParams(Socket* pSocket, void* pBuf, UINT uCount,
								Callback pCallback)
{
	IOPARAMS* pIO = new IOPARAMS;
	pIO->m_pSocket   = pSocket;
	pIO->m_pBuf      = pBuf;
	pIO->m_uCount    = uCount;
	pIO->m_pCallback = pCallback;
	return pIO;
}

IOPARAMS*
SocketState::createIOParams(Socket* pSocket, const void* pBuf, UINT uCount,
								Callback pCallback)
{
	IOPARAMS* pIO = new IOPARAMS;
	pIO->m_pSocket   = pSocket;
	pIO->m_pBuf      = (void *) pBuf;
	pIO->m_uCount    = uCount;
	pIO->m_pCallback = pCallback;
	return pIO;
}


// Remarks  : All of the subclasses of SocketState follow here.
//            The C++ Coding Standards states that each (sub)class
//            should be in a separate file.  For state tree classes
//            this is not desirable since it splits up the functionality
//            too much.

// Abstract : Returns the one (and only) instance of this object
//
// Returns  : Pointer to the SSClosed singleton object
// Params   :
//   -
//
// Pre      :
// Post     : If the singleton instance of this class did not already exist,
//            then it is allocated.
//
// Remarks  :
//
SocketState*
SSClosed::instance(void)
{
	if (m_pInstance == 0)
	{
		m_pInstance = new SSClosed;
	}
	return m_pInstance;
}


// Abstract : Returns the one (and only) instance of this object
//
// Returns  : Pointer to the SSOpenedServer singleton object
// Params   :
//   -
//
// Pre      :
// Post     : If the singleton instance of this class did not already exist,
//            then it is allocated.
//
// Remarks  :
//
SocketState*
SSOpenedServer::instance(void)
{
	if (m_pInstance == 0)
	{
		m_pInstance = new SSOpenedServer;
	}
	return m_pInstance;
}



// Abstract : Open a server socket as specified by a SocketAddr address
//
// Returns  : true on success
// Params   :
//   pSocket                   Pointer to socket object
//   rSockAddr                 Reference to a socket address to open
//   uOpenFlags                Flags indicating how the socket should be opened
//
// Pre      : This routine expects a well-formed SocketAddr.
// Post     : The open flags given as parameter are saved in member variables.
//            Upon successful completion of this routine, a server socket
//            will be opened and the object's state will be changed to
//            SSListening.  The application will need to call Listen in
//            order to receiving incoming connections via the socket to a client.
//
// Remarks  : Winsock (at least the implementation for Windows NT/95) does not
//            appreciate closing and re-opening a server socket.  Therefore,
//            an application using this class library should keep the server
//            socket open, and repeatedly call Socket::Listen() to accept
//            incoming clients.
//
bool
SSOpenedServer::open(Socket* pSocket,
                     SocketAddr& rSockAddr,
                     UINT  uOpenFlags)
{
	// Assume failure
	changeState(pSocket, SSClosed::instance());

	// Save the "file" open modes in object.
	// Save the non-standard modeAsyncSocket separately.
	pSocket->m_uOpenFlags =  uOpenFlags & ~Socket::modeAsyncSocket;
	pSocket->m_bAsyncMode = (uOpenFlags & Socket::modeAsyncSocket) ? true : false;

	// If broadcast (connectionless) then m_nProtocol is SOCK_DGRAM,
	//  else it is SOCK_STREAM.
	pSocket->m_hFile = ::socket(AF_INET, pSocket->m_nProtocol, 0);
	if (pSocket->m_hFile == INVALID_SOCKET)
		return false;

	// TODO: Maybe skip the sockopt if portnum = 0?
	bool bSockOpt = true;
	::setsockopt(pSocket->m_hFile, SOL_SOCKET, SO_REUSEADDR,
				 (char *)&bSockOpt, sizeof(bool));

	if (pSocket->m_nProtocol == SOCK_STREAM)
	{
#ifdef TARGET_WINDOWS
		bool bSockOpt = true;
		::setsockopt(pSocket->m_hFile, SOL_SOCKET, SO_KEEPALIVE,
					 (char *)&bSockOpt, sizeof(bool));
#else
        int soopt = 1;
		::setsockopt(pSocket->m_hFile, SOL_SOCKET, SO_KEEPALIVE,
					 &soopt, sizeof(soopt));
#endif
	}

	if (::bind(pSocket->m_hFile, (sockaddr *)rSockAddr, sizeof(sockaddr)) == SOCKET_ERROR)
    {
        close(pSocket);
		return false;
    }

	if (pSocket->m_nProtocol == SOCK_STREAM)
	{
		if (::listen(pSocket->m_hFile, SOMAXCONN) == SOCKET_ERROR)
        {
            close(pSocket);
			return false;
        }

		// Open was successful -- next state
		changeState(pSocket, SSListening::instance());
	}
	else	// SOCK_DGRAM
	{
		bSockOpt = true;
		::setsockopt(pSocket->m_hFile, SOL_SOCKET, SO_BROADCAST,
					 (char *)&bSockOpt, sizeof(bool));

		rSockAddr.sin_addr.s_addr = INADDR_BROADCAST;

		changeState(pSocket, SSConnected::instance());
	}

    return true;
}


// Abstract : Returns the one (and only) instance of this object
//
// Returns  : Pointer to the SSOpenedClient singleton object
// Params   :
//   -
//
// Pre      :
// Post     : If the singleton instance of this class did not already exist,
//            then it is allocated.
//
// Remarks  :
//
SocketState*
SSOpenedClient::instance(void)
{
	if (m_pInstance == 0)
	{
		m_pInstance = new SSOpenedClient;
	}
	return m_pInstance;
}



// Abstract : Open a socket as specified by a SocketAddr address
//
// Returns  : true on success
// Params   :
//   pSocket                   Pointer to socket object
//   rSockAddr                 Reference to a socket address to open
//   uOpenFlags                Flags indicating how the socket should be opened
//
// Pre      : This routine expects a well-formed SocketAddr.
// Post     : The open flags given as parameter are saved in member variables.
//            Upon successful completion of this routine, a client socket
//            will be opened and the object's state will be changed to
//            SSConnected.
//
// Remarks  :
//
bool
SSOpenedClient::open(Socket* pSocket,
                     SocketAddr& rSockAddr,
                     UINT  uOpenFlags)
{
	// Assume failure
	changeState(pSocket, SSClosed::instance());

	// Save the "file" open modes in object.  Save the non-standard
	// modeAsyncSocket separately.
	pSocket->m_uOpenFlags = uOpenFlags & ~Socket::modeAsyncSocket;
	pSocket->m_bAsyncMode = (uOpenFlags & Socket::modeAsyncSocket) ? true : false;

	// If broadcast (connectionless) then m_nProtocol is SOCK_DGRAM,
	//  else it is SOCK_STREAM.
	pSocket->m_hFile = ::socket(AF_INET, pSocket->m_nProtocol, 0);
	if (pSocket->m_hFile == INVALID_SOCKET)
		return false;

	if (pSocket->m_nProtocol == SOCK_STREAM)
	{
		if (::connect(pSocket->m_hFile, (sockaddr *)rSockAddr, sizeof(sockaddr))
			== SOCKET_ERROR)
			return false;

//        std::cout << "2 SSOpenedClient::open errno=" << errno << std::endl;
#ifdef TARGET_WINDOWS
		bool bSockOpt = true;
		::setsockopt(pSocket->m_hFile, SOL_SOCKET, SO_KEEPALIVE,
					 (char *)&bSockOpt, sizeof(bool));
#else
        int soopt = 1;
		::setsockopt(pSocket->m_hFile, SOL_SOCKET, SO_KEEPALIVE,
					 &soopt, sizeof(soopt));
#endif
//        std::cout << "3 SSOpenedClient::open errno=" << errno << std::endl;
	}
	else
	{
		// For UDP, client needs to bind with port 0.
		sockaddr_in BindAddr;
		BindAddr.sin_family      = AF_INET;
		BindAddr.sin_addr.s_addr = INADDR_ANY;
		BindAddr.sin_port        = 0;

		if (::bind(pSocket->m_hFile, (sockaddr *)&BindAddr, sizeof(sockaddr_in))
			== SOCKET_ERROR)
			return false;

		bool bSockOpt = true;
		::setsockopt(pSocket->m_hFile, SOL_SOCKET, SO_BROADCAST,
					 (char *)&bSockOpt, sizeof(bool));

		rSockAddr.sin_addr.s_addr = INADDR_BROADCAST;
	}

	// Open was successful -- next state
	changeState(pSocket, SSConnected::instance());
	return true;
}



#if USE_OPENSSL
// Abstract : Returns the one (and only) instance of this object
//
// Returns  : Pointer to the SSOpenedClientTLS singleton object
// Params   :
//   -
//
// Pre      :
// Post     : If the singleton instance of this class did not already exist,
//            then it is allocated.
//
// Remarks  :
//
SocketState*
SSOpenedClientTLS::instance(void)
{
	if (m_pInstance == 0)
	{
		m_pInstance = new SSOpenedClientTLS;
	}
	return m_pInstance;
}


// Abstract : Open a socket as specified by a SocketAddr address
//
// Returns  : true on success
// Params   :
//   pSocket                   Pointer to socket object
//   rSockAddr                 Reference to a socket address to open
//   uOpenFlags                Flags indicating how the socket should be opened
//
// Pre      : This routine expects a well-formed SocketAddr.
// Post     : The open flags given as parameter are saved in member variables.
//            Upon successful completion of this routine, a client socket
//            will be opened and the object's state will be changed to
//            SSConnected.
//
// Remarks  :
//
bool
SSOpenedClientTLS::open(Socket* pSocket,
                        SocketAddr& rSockAddr,
                        UINT  uOpenFlags)
{
	// Assume failure
	changeState(pSocket, SSClosed::instance());

	// Save the "file" open modes in object.  Save the non-standard
	// modeAsyncSocket separately.
	pSocket->m_uOpenFlags = uOpenFlags & ~Socket::modeAsyncSocket;
	pSocket->m_bAsyncMode = (uOpenFlags & Socket::modeAsyncSocket) ? true : false;

    SSL_library_init();
    SSL_load_error_strings();
    //TODO: set bio_err = BIO_new_fp(stderr, BIO_NOCLOSE);

    const SSL_METHOD* meth = SSLv23_method();
    SSL_CTX* ctx = SSL_CTX_new(meth);

	// If broadcast (connectionless) then m_nProtocol is SOCK_DGRAM,
	//  else it is SOCK_STREAM.
	pSocket->m_hFile = ::socket(AF_INET, pSocket->m_nProtocol, 0);
	if (pSocket->m_hFile == INVALID_SOCKET)
		return false;

	if (pSocket->m_nProtocol == SOCK_STREAM)
	{
		if (::connect(pSocket->m_hFile, (sockaddr *)rSockAddr, sizeof(sockaddr))
			== SOCKET_ERROR)
        {
            close(pSocket);
			return false;
        }

//        std::cout << "2 SSOpenedClientTLS::open errno=" << errno << std::endl;
#ifdef TARGET_WINDOWS
		bool bSockOpt = true;
#else
		int bSockOpt = 1;
#endif
		::setsockopt(pSocket->m_hFile, SOL_SOCKET, SO_KEEPALIVE,
					 (char *)&bSockOpt, sizeof(bSockOpt));
//        std::cout << "3 SSOpenedClientTLS::open errno=" << errno << std::endl;
	}
	else
	{
		// Cannot do UDP on a secure socket
        close(pSocket);
        return false;
	}

	// Open was successful -- next state
	changeState(pSocket, SSConnectedTLS::instance());
	return true;
}


// Abstract : Returns the one (and only) instance of this object
//
// Returns  : Pointer to the SSConnectedTLS singleton object
// Params   :
//   -
//
// Pre      :
// Post     : If the singleton instance of this class did not already exist,
//            then it is allocated.
//
// Remarks  :
//
SocketState*
SSConnectedTLS::instance(void)
{
	if (m_pInstance == 0)
	{
		m_pInstance = new SSConnectedTLS;
	}
	return m_pInstance;
}

// Abstract : Closes the socket connection
//
// Returns  : -
// Params   :
//   pSocket                   Pointer to socket object
//
// Pre      :
// Post     : The socket connection is closed if it was open.  The pSocket
//            object is switched to a closed state.
//
// Remarks  :
//
void
SSConnectedTLS::close(Socket* pSocket)
{
    //TODO: close down the SSL

    SocketState::close(pSocket);
}


// Abstract : Read in a specified number of raw bytes from socket
//
// Returns  : UINT (actual number of bytes read)
// Params   :
//   pSocket                   Pointer to socket object
//   pBuf                      Buffer that will be filled
//   uCount                    Size of buffer
//
// Pre      : The flags specified when the socket was opened must allow
//            reading.  The number of byte to read must not be zero.
// Post     : This routine will fill pBuf with data that is available
//            from the socket.  It will read no more than the maximum
//            given by the uCount parameter.  This routine returns the
//            number of bytes actually read.
//            If the socket is in asynchronous mode and no data is
//            immediately available, then a read worker thread will be created.
//
// Remarks  : This routine supports synchronous (blocking), asynchronous
//            (overlapped), and polling modes of reading from the socket.
//            See Table 2, "Socket I/O Modes" from the reference manual for
//            an overview of possible I/O modes that this routine implements.
//            Note that it is vital to not call the destructor of a
//            Socket object while it has active worker threads.  It is
//            best to always call Close() on the socket before deleting it.
//
UINT
SSConnectedTLS::read(Socket* pSocket, void* pBuf, UINT uCount)
{
	int iResult = 0;

	// Error: this socket is write-only
	VERIFY(!(pSocket->m_uOpenFlags & Socket::modeWrite));
	// Error: trying to read 0 bytes
	VERIFY(uCount != 0);

	if (! pSocket->m_bAsyncMode)
	{
		// Synchronous mode -- do a blocking read on socket
		iResult = readSocket(pSocket, pBuf, uCount);
		if (iResult == 0 || iResult == SOCKET_ERROR)
        {
            pSocket->m_Status = SC_NODATA;
            pSocket->setstate(std::ios::eofbit);
			return 0;
        }
        else
            pSocket->clear(pSocket->rdstate() & ~std::ios::eofbit);	}
	else
	{
		// Asynchronous mode -- if data is available on socket then read
		// it.  Otherwise, set up a reader thread to wait for data.
		DWORD dwBytes;
	    if (IOCTLSOCK(pSocket->m_hFile, FIONREAD, &dwBytes) == SOCKET_ERROR)
        {
#ifdef TARGET_WINDOWS
                WSAGetLastError();
#endif
                // WSAEINPROGRESS means a blocking call is still active.  Maybe
                // in the future take action on this status?
                pSocket->m_Status = SC_NODATA;
                pSocket->setstate(std::ios::eofbit);
                return 0;
        }

		// avoid blocking the main thread
		if (dwBytes /*&& ! WSAIsBlocking()*/)	// This much (dwBytes) can be
		{										// read without blocking
#ifdef TARGET_WINDOWS
			iResult = readSocket(pSocket, pBuf, min((UINT)dwBytes, uCount));
#else
			iResult = readSocket(pSocket, pBuf, std::min((UINT)dwBytes, uCount));
#endif
			if (iResult == 0 || iResult == SOCKET_ERROR)
            {
                pSocket->m_Status = SC_NODATA;
                pSocket->setstate(iResult == SOCKET_ERROR 
                                   ? std::ios::badbit : std::ios::eofbit);
				return 0;
            }
		}
		else
		{
			if (pSocket->m_pDefCallback == 0)
			{
                pSocket->m_Status = SC_NODATA;
                pSocket->setstate(std::ios::eofbit);
				return 0;
			}
#if CONFIG_HAS_PTHREADS
            pthread_t thread_id;
            int st = pthread_create(&thread_id, NULL, Socket::readerThread, 
								  createIOParams(pSocket, pBuf, uCount,
                                                 pSocket->m_pDefCallback));
            VERIFY(st==0);
#else
#ifdef WIN32
#ifdef USE_MFC
			VERIFY(AfxBeginThread((AFX_THREADPROC) Socket::readerThread,
								  createIOParams(pSocket, pBuf, uCount,
								  pSocket->m_pDefCallback)));
#else
			DWORD thread_id;
			VERIFY(CreateThread( 
            NULL,                   // default security attributes
            0,                      // use default stack size  
			Socket::readerThread,   // thread function name
            createIOParams(pSocket, pBuf, uCount,
						   pSocket->m_pDefCallback),          // argument to thread function 
            0,                      // use default creation flags 
            &thread_id));   // returns the thread identifier 
#endif
#else
			// multi-threaded mode not implemented for this platform
			VERIFY(0);
#endif
#endif
		}
	}

    if (iResult >= 0)
    {
        pSocket->m_Status = SC_OK;
        pSocket->clear();
    }

	return iResult;
}


// Abstract : Internal "helper" function to read in a specified number of bytes
//            from a socket.  This function always uses blocking I/O.
//
// Returns  : int (actual number of bytes read)
// Params   :
//   pSocket                   Pointer to socket object
//   pBuf                      Buffer that will be filled
//   uCount                    Size of buffer
//
// Pre      : The flags specified when the socket was opened must allow
//            reading.
// Post     : This routine will fill pBuf with data that is available
//            from the socket.  It will read no more than the maximum
//            given by the uCount parameter.  This routine returns the
//            number of bytes actually read.
//
// Remarks  : This function uses the Winsock function ::recv to read data from
//            a TCP/IP socket, and uses the Winsock function ::recvfrom to read
//            in an UDP datagram.
int
SSConnectedTLS::readSocket(Socket* pSocket, void* pBuf, UINT uCount)
{
	int iResult = 0;

	if (pSocket->m_nProtocol == SOCK_DGRAM)
	{
		socklen_t iSizeFrom = sizeof(sockaddr_in);
		iResult = ::recvfrom(pSocket->m_hFile, (char *)pBuf, uCount,
							 0, (sockaddr *) &pSocket->m_PeerAddr, &iSizeFrom);
	}
	else
	{
		iResult = ::recv(pSocket->m_hFile, (char *)pBuf, uCount, 0);
	}

	return iResult;
}


// Abstract : Process for the read worker thread
//
// Returns  : 0 if successful, 1 if an error occurred
// Params   :
//   pIOP                      Pointer to the I/O parameters
//
// Pre      : The socket connection has already been established and the
//            Socket object is set to asynchronous I/O mode.
// Post     : The user's registered callback routine is called with the
//            number of bytes that were read and the I/O parameters (which
//            includes the buffer.
//
// Remarks  : The user's callback routine is NOT called if no bytes were read
//            or an error occurred.  Note that recv() returns a SOCKET_ERROR
//            in the case that the socket was closed (perhaps by another
//            thread).  This is deemed normal behavior for an application
//            that wants to terminate all of its outstanding worker threads
//            before quitting.  That is why the decision was made to NOT call
//            the callback in these situations.
//
DWORD
SSConnectedTLS::readerThread(IOPARAMS* pIOP)
{
	int iResult;

	iResult = readSocket(pIOP->m_pSocket, pIOP->m_pBuf, pIOP->m_uCount);
	if (iResult == 0 || iResult == SOCKET_ERROR)
		return 1;			// Thread exit code 1 == failure

	pIOP->m_pCallback(iResult, pIOP->m_pBuf);

	return 0;		// Return success
}


// Abstract : Write the specified number of raw bytes from the user's buffer
//            to the socket.
//
// Returns  : -
// Params   :
//   pBuf                      Buffer that will be written
//   uCount                    Number of bytes to write
//
// Pre      : The flags specified when the socket was opened must allow
//            writing.
// Post     : This routine will write uCount bytes of data from pBuf to
//            the socket.
//
// Post     : This routine will fill pBuf with data that is available
//            from the socket.  It will read no more than the maximum
//            given by the uCount parameter.  This routine returns the
//            number of bytes actually read.
//            If the socket is in asynchronous mode, then a write worker thread
//            will be created.
//
// Remarks  : This routine supports synchronous (blocking), and asynchronous
//            (overlapped) modes of writing from the socket.
//            See Table 2, "Socket I/O Modes" from IPC document for
//            an overview of of possible I/O modes that this routine implements.
//            Note that it is vital to not call the destructor of an
//            Socket object while it has active worker threads.  It is
//            best to always call Close() on the socket before deleting it.
//
void
SSConnectedTLS::write(Socket* pSocket, const void* pBuf, UINT uCount)
{
	// Error: this socket is read-only
	VERIFY(pSocket->m_uOpenFlags & (Socket::modeReadWrite | Socket::modeWrite));

	if (! (pSocket->m_bAsyncMode && pSocket->m_pDefCallback != 0))
	{
		int iResult;
		// Synchronous mode -- do a blocking write on socket
		if (pSocket->m_nProtocol == SOCK_DGRAM)
		{
			// Note that s_addr could have been overwritten by the call to recvfrom
			pSocket->m_PeerAddr.sin_addr.s_addr = INADDR_BROADCAST;
			iResult = ::sendto(pSocket->m_hFile, (const char *)pBuf, uCount, 0,
				         (sockaddr *)&pSocket->m_PeerAddr, sizeof(pSocket->m_PeerAddr));
		}
		else
		{
			iResult = ::send(pSocket->m_hFile, (const char *)pBuf, uCount, 0);
		}
		if (iResult == SOCKET_ERROR)
        {
			pSocket->m_Status = SC_FAILED;
            pSocket->setstate(std::ios::failbit);
        }
        else
            pSocket->clear(pSocket->rdstate() & ~std::ios::failbit);
	}
	else
	{
#if CONFIG_HAS_PTHREADS
		// Asynchronous mode -- let a worker thread write and wait.
        pthread_t thread_id;
        int st = pthread_create(&thread_id, NULL, Socket::writerThread, 
                                createIOParams(pSocket, pBuf, uCount,
                                               pSocket->m_pDefCallback));
        VERIFY(st==0);
#else
#ifdef WIN32
#ifdef USE_MFC
		VERIFY(AfxBeginThread((AFX_THREADPROC) Socket::writerThread,
							  createIOParams(pSocket, pBuf, uCount,
							  pSocket->m_pDefCallback)));
#else
			DWORD thread_id;
			VERIFY(CreateThread( 
            NULL,                   // default security attributes
            0,                      // use default stack size  
			Socket::writerThread,   // thread function name
            createIOParams(pSocket, pBuf, uCount,
						   pSocket->m_pDefCallback),          // argument to thread function 
            0,                      // use default creation flags 
            &thread_id));   // returns the thread identifier 
#endif
#else
        // multi-threaded not implemented for this platform
		VERIFY(0);
#endif
#endif
	}
}


// Abstract : Process for the write worker thread
//
// Returns  : 0 if successful, 1 if an error occurred
// Params   :
//   pIOP                      Pointer to the I/O parameters
//
// Pre      : The socket connection has already been established and the
//            Socket object is set to asynchronous I/O mode.
// Post     : The user's registered callback routine is called with the
//            number of bytes that were written and the I/O parameters (which
//            includes the buffer).
//
// Remarks  : The user's callback routine is NOT called if no bytes were read
//            or an error occurred.  Note that recv() returns a SOCKET_ERROR
//            in the case that the socket was closed (perhaps by another
//            thread).  This is deemed normal behavior for an application
//            that wants to terminate all of its outstanding worker threads
//            before quitting.  That is why the decision was made to NOT call
//            the callback in these situations.
//
DWORD
SSConnectedTLS::writerThread(IOPARAMS* pIOP)
{
	int iResult;
	if (pIOP->m_pSocket->m_nProtocol == SOCK_DGRAM)
	{
		// Note that s_addr could have been overwritten by the call to recvfrom
		pIOP->m_pSocket->m_PeerAddr.sin_addr.s_addr = INADDR_BROADCAST;
		iResult = ::sendto(pIOP->m_pSocket->m_hFile, (const char *)pIOP->m_pBuf,
						   pIOP->m_uCount, 0,
				           (sockaddr *)&pIOP->m_pSocket->m_PeerAddr,
						   sizeof(pIOP->m_pSocket->m_PeerAddr));
	}
	else
	{
		iResult = ::send(pIOP->m_pSocket->m_hFile, (const char *)pIOP->m_pBuf,
						 pIOP->m_uCount, 0);
	}
	if (iResult == SOCKET_ERROR)
		return 1;			// Thread exit code 1 == failure

	pIOP->m_pCallback(iResult, pIOP->m_pBuf);

	return 0;
}

#endif // USE_OPENSSL

// Abstract : Returns the one (and only) instance of this object
//
// Returns  : Pointer to the SSListening singleton object
// Params   :
//   -
//
// Pre      :
// Post     : If the singleton instance of this class did not already exist,
//            then it is allocated.
//
// Remarks  :
//
SocketState*
SSListening::instance(void)
{
	if (m_pInstance == 0)
	{
		m_pInstance = new SSListening;
	}
	return m_pInstance;
}


// Abstract : Listen on a server-side socket for incoming connection requests
//
// Returns  : Socket handle of incoming connection
// Params   :
//   pSocket                   Pointer to socket object
//   nBackLog                  Maximum number of clients that can be
//                             queued waiting for a connection
//
// Pre      :
// Post     : Upon successful completion of this routine, a new socket handle
//            is created which is connected to a client.  The original server
//            socket remains in listen mode.
//
//
SOCKET
SSListening::listen(Socket* pSocket, const int /*nBacklog*/)
{
	SOCKET hSock;
	if ((hSock = ::accept(pSocket->m_hFile, 0, 0)) == INVALID_SOCKET)
		return INVALID_SOCKET;

	return hSock;
}


// Abstract : Returns the one (and only) instance of this object
//
// Returns  : Pointer to the SSConnected singleton object
// Params   :
//   -
//
// Pre      :
// Post     : If the singleton instance of this class did not already exist,
//            then it is allocated.
//
// Remarks  :
//
SocketState*
SSConnected::instance(void)
{
	if (m_pInstance == 0)
	{
		m_pInstance = new SSConnected;
	}
	return m_pInstance;
}


// Abstract : Read in a specified number of raw bytes from socket
//
// Returns  : UINT (actual number of bytes read)
// Params   :
//   pSocket                   Pointer to socket object
//   pBuf                      Buffer that will be filled
//   uCount                    Size of buffer
//
// Pre      : The flags specified when the socket was opened must allow
//            reading.  The number of byte to read must not be zero.
// Post     : This routine will fill pBuf with data that is available
//            from the socket.  It will read no more than the maximum
//            given by the uCount parameter.  This routine returns the
//            number of bytes actually read.
//            If the socket is in asynchronous mode and no data is
//            immediately available, then a read worker thread will be created.
//
// Remarks  : This routine supports synchronous (blocking), asynchronous
//            (overlapped), and polling modes of reading from the socket.
//            See Table 2, "Socket I/O Modes" from the reference manual for
//            an overview of possible I/O modes that this routine implements.
//            Note that it is vital to not call the destructor of a
//            Socket object while it has active worker threads.  It is
//            best to always call Close() on the socket before deleting it.
//
UINT
SSConnected::read(Socket* pSocket, void* pBuf, UINT uCount)
{
	int iResult = 0;

	// Error: this socket is write-only
	VERIFY(!(pSocket->m_uOpenFlags & Socket::modeWrite));
	// Error: trying to read 0 bytes
	VERIFY(uCount != 0);

	if (! pSocket->m_bAsyncMode)
	{
		// Synchronous mode -- do a blocking read on socket
		iResult = readSocket(pSocket, pBuf, uCount);
		if (iResult == 0 || iResult == SOCKET_ERROR)
        {
            pSocket->m_Status = SC_NODATA;
            pSocket->setstate(std::ios::eofbit);
			return 0;
        }
        else
            pSocket->clear(pSocket->rdstate() & ~std::ios::eofbit);	}
	else
	{
		// Asynchronous mode -- if data is available on socket then read
		// it.  Otherwise, set up a reader thread to wait for data.
		DWORD dwBytes;
	    if (IOCTLSOCK(pSocket->m_hFile, FIONREAD, &dwBytes) == SOCKET_ERROR)
        {
#ifdef TARGET_WINDOWS
                WSAGetLastError();
#endif
                // WSAEINPROGRESS means a blocking call is still active.  Maybe
                // in the future take action on this status?
                pSocket->m_Status = SC_NODATA;
                pSocket->setstate(std::ios::eofbit);
                return 0;
        }

		// avoid blocking the main thread
		if (dwBytes /*&& ! WSAIsBlocking()*/)	// This much (dwBytes) can be
		{										// read without blocking
#ifdef TARGET_WINDOWS
			iResult = readSocket(pSocket, pBuf, min((UINT)dwBytes, uCount));
#else
			iResult = readSocket(pSocket, pBuf, std::min((UINT)dwBytes, uCount));
#endif
			if (iResult == 0 || iResult == SOCKET_ERROR)
            {
                pSocket->m_Status = SC_NODATA;
                pSocket->setstate(iResult == SOCKET_ERROR 
                                   ? std::ios::badbit : std::ios::eofbit);
				return 0;
            }
		}
		else
		{
			if (pSocket->m_pDefCallback == 0)
			{
                pSocket->m_Status = SC_NODATA;
                pSocket->setstate(std::ios::eofbit);
				return 0;
			}
#if CONFIG_HAS_PTHREADS
            pthread_t thread_id;
            int st = pthread_create(&thread_id, NULL, Socket::readerThread, 
								  createIOParams(pSocket, pBuf, uCount,
                                                 pSocket->m_pDefCallback));
            VERIFY(st==0);
#else
#ifdef WIN32
#ifdef USE_MFC
			VERIFY(AfxBeginThread((AFX_THREADPROC) Socket::readerThread,
								  createIOParams(pSocket, pBuf, uCount,
								  pSocket->m_pDefCallback)));
#else
			DWORD thread_id;
			VERIFY(CreateThread( 
            NULL,                   // default security attributes
            0,                      // use default stack size  
			Socket::readerThread,   // thread function name
            createIOParams(pSocket, pBuf, uCount,
						   pSocket->m_pDefCallback),          // argument to thread function 
            0,                      // use default creation flags 
            &thread_id));   // returns the thread identifier 
#endif
#else
			// multi-threaded mode not implemented for this platform
			VERIFY(0);
#endif
#endif
		}
	}

    if (iResult >= 0)
    {
        pSocket->m_Status = SC_OK;
        pSocket->clear();
    }

	return iResult;
}


// Abstract : Internal "helper" function to read in a specified number of bytes
//            from a socket.  This function always uses blocking I/O.
//
// Returns  : int (actual number of bytes read)
// Params   :
//   pSocket                   Pointer to socket object
//   pBuf                      Buffer that will be filled
//   uCount                    Size of buffer
//
// Pre      : The flags specified when the socket was opened must allow
//            reading.
// Post     : This routine will fill pBuf with data that is available
//            from the socket.  It will read no more than the maximum
//            given by the uCount parameter.  This routine returns the
//            number of bytes actually read.
//
// Remarks  : This function uses the Winsock function ::recv to read data from
//            a TCP/IP socket, and uses the Winsock function ::recvfrom to read
//            in an UDP datagram.
int
SSConnected::readSocket(Socket* pSocket, void* pBuf, UINT uCount)
{
	int iResult = 0;

	if (pSocket->m_nProtocol == SOCK_DGRAM)
	{
		socklen_t iSizeFrom = sizeof(sockaddr_in);
		iResult = ::recvfrom(pSocket->m_hFile, (char *)pBuf, uCount,
							 0, (sockaddr *) &pSocket->m_PeerAddr, &iSizeFrom);
	}
	else
	{
		iResult = ::recv(pSocket->m_hFile, (char *)pBuf, uCount, 0);
	}

	return iResult;
}


// Abstract : Process for the read worker thread
//
// Returns  : 0 if successful, 1 if an error occurred
// Params   :
//   pIOP                      Pointer to the I/O parameters
//
// Pre      : The socket connection has already been established and the
//            Socket object is set to asynchronous I/O mode.
// Post     : The user's registered callback routine is called with the
//            number of bytes that were read and the I/O parameters (which
//            includes the buffer.
//
// Remarks  : The user's callback routine is NOT called if no bytes were read
//            or an error occurred.  Note that recv() returns a SOCKET_ERROR
//            in the case that the socket was closed (perhaps by another
//            thread).  This is deemed normal behavior for an application
//            that wants to terminate all of its outstanding worker threads
//            before quitting.  That is why the decision was made to NOT call
//            the callback in these situations.
//
DWORD
SSConnected::readerThread(IOPARAMS* pIOP)
{
	int iResult;

	iResult = readSocket(pIOP->m_pSocket, pIOP->m_pBuf, pIOP->m_uCount);
	if (iResult == 0 || iResult == SOCKET_ERROR)
		return 1;			// Thread exit code 1 == failure

	pIOP->m_pCallback(iResult, pIOP->m_pBuf);

	return 0;		// Return success
}


// Abstract : Write the specified number of raw bytes from the user's buffer
//            to the socket.
//
// Returns  : -
// Params   :
//   pBuf                      Buffer that will be written
//   uCount                    Number of bytes to write
//
// Pre      : The flags specified when the socket was opened must allow
//            writing.
// Post     : This routine will write uCount bytes of data from pBuf to
//            the socket.
//
// Post     : This routine will fill pBuf with data that is available
//            from the socket.  It will read no more than the maximum
//            given by the uCount parameter.  This routine returns the
//            number of bytes actually read.
//            If the socket is in asynchronous mode, then a write worker thread
//            will be created.
//
// Remarks  : This routine supports synchronous (blocking), and asynchronous
//            (overlapped) modes of writing from the socket.
//            See Table 2, "Socket I/O Modes" from IPC document for
//            an overview of of possible I/O modes that this routine implements.
//            Note that it is vital to not call the destructor of an
//            Socket object while it has active worker threads.  It is
//            best to always call Close() on the socket before deleting it.
//
void
SSConnected::write(Socket* pSocket, const void* pBuf, UINT uCount)
{
	// Error: this socket is read-only
	VERIFY(pSocket->m_uOpenFlags & (Socket::modeReadWrite | Socket::modeWrite));

	if (! (pSocket->m_bAsyncMode && pSocket->m_pDefCallback != 0))
	{
		int iResult;
		// Synchronous mode -- do a blocking write on socket
		if (pSocket->m_nProtocol == SOCK_DGRAM)
		{
			// Note that s_addr could have been overwritten by the call to recvfrom
			pSocket->m_PeerAddr.sin_addr.s_addr = INADDR_BROADCAST;
			iResult = ::sendto(pSocket->m_hFile, (const char *)pBuf, uCount, 0,
				         (sockaddr *)&pSocket->m_PeerAddr, sizeof(pSocket->m_PeerAddr));
		}
		else
		{
			iResult = ::send(pSocket->m_hFile, (const char *)pBuf, uCount, 0);
		}
		if (iResult == SOCKET_ERROR)
        {
			pSocket->m_Status = SC_FAILED;
            pSocket->setstate(std::ios::failbit);
        }
        else
            pSocket->clear(pSocket->rdstate() & ~std::ios::failbit);
	}
	else
	{
#if CONFIG_HAS_PTHREADS
		// Asynchronous mode -- let a worker thread write and wait.
        pthread_t thread_id;
        int st = pthread_create(&thread_id, NULL, Socket::writerThread, 
                                createIOParams(pSocket, pBuf, uCount,
                                               pSocket->m_pDefCallback));
        VERIFY(st==0);
#else
#ifdef WIN32
#ifdef USE_MFC
		VERIFY(AfxBeginThread((AFX_THREADPROC) Socket::writerThread,
							  createIOParams(pSocket, pBuf, uCount,
							  pSocket->m_pDefCallback)));
#else
			DWORD thread_id;
			VERIFY(CreateThread( 
            NULL,                   // default security attributes
            0,                      // use default stack size  
			Socket::writerThread,   // thread function name
            createIOParams(pSocket, pBuf, uCount,
						   pSocket->m_pDefCallback),          // argument to thread function 
            0,                      // use default creation flags 
            &thread_id));   // returns the thread identifier 
#endif
#else
        // multi-threaded not implemented for this platform
		VERIFY(0);
#endif
#endif
	}
}


// Abstract : Process for the write worker thread
//
// Returns  : 0 if successful, 1 if an error occurred
// Params   :
//   pIOP                      Pointer to the I/O parameters
//
// Pre      : The socket connection has already been established and the
//            Socket object is set to asynchronous I/O mode.
// Post     : The user's registered callback routine is called with the
//            number of bytes that were written and the I/O parameters (which
//            includes the buffer).
//
// Remarks  : The user's callback routine is NOT called if no bytes were read
//            or an error occurred.  Note that recv() returns a SOCKET_ERROR
//            in the case that the socket was closed (perhaps by another
//            thread).  This is deemed normal behavior for an application
//            that wants to terminate all of its outstanding worker threads
//            before quitting.  That is why the decision was made to NOT call
//            the callback in these situations.
//
DWORD
SSConnected::writerThread(IOPARAMS* pIOP)
{
	int iResult;
	if (pIOP->m_pSocket->m_nProtocol == SOCK_DGRAM)
	{
		// Note that s_addr could have been overwritten by the call to recvfrom
		pIOP->m_pSocket->m_PeerAddr.sin_addr.s_addr = INADDR_BROADCAST;
		iResult = ::sendto(pIOP->m_pSocket->m_hFile, (const char *)pIOP->m_pBuf,
						   pIOP->m_uCount, 0,
				           (sockaddr *)&pIOP->m_pSocket->m_PeerAddr,
						   sizeof(pIOP->m_pSocket->m_PeerAddr));
	}
	else
	{
		iResult = ::send(pIOP->m_pSocket->m_hFile, (const char *)pIOP->m_pBuf,
						 pIOP->m_uCount, 0);
	}
	if (iResult == SOCKET_ERROR)
		return 1;			// Thread exit code 1 == failure

	pIOP->m_pCallback(iResult, pIOP->m_pBuf);

	return 0;
}

//
// END OF FILE
//
