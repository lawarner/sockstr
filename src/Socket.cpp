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
// File       : Socket.cpp
//
// Class      : Socket
//
// Description: The Socket class provides a transport-specific
//              interface for IPC based on Windows Sockets.
//
// Decisions  : This class inherits from Stream and therefore provides
//              an interface that is based on the MFC CFile class.  A
//              user application could further derive sub-classes from
//              Socket to do application specific processing, but this is
//              probably not desirable since this might break the Stream
//              polymorphic interface.  An application can better include
//              references to Stream objects within its class to
//              provide new functionality.
//

//
// INCLUDE FILES
//
#include "config.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#ifdef linux
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#else
#include <WinSock2.h>
#include <WS2tcpip.h>
#endif
#include <sockstr/IPC.h>
#include <sockstr/Socket.h>
#include <sockstr/SocketState.h>

using namespace sockstr;

//
// MACRO DEFINITIONS
//
#ifdef _WINDOWS
#pragma warning(disable : 4244)  // Disable warning message for atoi()
#endif

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
DWORD  Socket::m_dwSequence  = 0;
#ifdef _DEBUG
void* Socket::m_pLastBuffer = 0;
#endif
const int Socket::modeCreate       = 1;
const int Socket::modeAsyncSocket  = 2;
const int Socket::modeRead         = 4;
const int Socket::modeWrite        = 8;
const int Socket::modeReadWrite    = 16;


//
// CLASS MEMBER FUNCTION DEFINITIONS
//

// Abstract : Constructs a Socket object
//
// Returns  : -
// Params   :
//   lpszFileName              Fully qualified host name and port
//   uOpenFlags                open mode flags
//   rSockAddr                 An already resolved address
//
// Pre      :
// Post     : The default constructor creates the object in a closed
//            state.  Thus the user will need to subsequently call
//            the Open() member function.  The other 2 constructors
//            attempt to open a (client or server) socket connection.
//            In any case the object's status will be set according to
//            success or failure.
//
// Remarks  :
//
Socket::Socket(void)
{
	initialize();	// Do setup common to all constructors
}


Socket::Socket(const char* lpszFileName, UINT uOpenFlags)
{
	initialize();	// Do setup common to all constructors
	if (!open(lpszFileName, uOpenFlags))
	{
		m_Status = SC_FAILED;
        setstate(std::ios::failbit);
	}
}


Socket::Socket(SocketAddr& rSockAddr, UINT uOpenFlags)
{
	initialize();	// Do setup common to all constructors
	if (!open(rSockAddr, uOpenFlags))
	{
		m_Status = SC_FAILED;
        setstate(std::ios::failbit);
	}
}


// Abstract : Destructor of Socket object
//
// Returns  : -
// Params   :
//   -
//
// Pre      :
// Post     : The Socket object is destructed.  The socket
//            connection will be closed if it was still open.
//
// Remarks  :
//
Socket::~Socket(void)
{
	close();	// Just in case connection is still open
}


// Abstract : Assignment operator
//
// Returns  : Reference to Socket object that was assigned
// Params   :
//   rSource                   Reference to a Socket object from which the
//                             variables are copied.
//
// Pre      :
// Post     : The internal member variables of the assignee (Socket that
//            is to the left of the equal sign are set equal to the value 
//            contained in rSource.
//
// Remarks  :
//
Socket&
Socket::operator=(const Socket& rSource)
{
//    Stream::operator=(rSource);

	m_hFile = rSource.m_hFile;
	m_uOpenFlags = rSource.m_uOpenFlags;
	m_bAsyncMode = rSource.m_bAsyncMode;

	m_pState = rSource.m_pState;
	m_Status = rSource.m_Status;
	m_PeerAddr = rSource.m_PeerAddr;
	return *this;
}


// Abstract : Perform common initialization for Socket object.
//
// Returns  : -
// Params   :
//   -
//
// Pre      :
// Post     : The initial state of the Socket object is set to
//            "closed".  Also, asynchronous mode is turned off (blocking
//            mode is the default).
//
// Remarks  : This routine is only used internally by the Socket
//            class.  It does common initializations for all of
//            the constructors.
//
void
Socket::initialize(void)
{
	m_bAsyncMode = false;
	// Set initial state to Closed
	m_pState = SSClosed::instance();
}


// Abstract : Set I/O mode to Synchronous or Asynchronous
//
// Returns  : -
// Params   :
//   bMode                    Synchronous or Asynchronous I/O mode
//
// Pre      :
// Post     : If the parameter is TRUE then all subsequent I/O on the
//            socket will be performed asynchronously.  Otherwise,
//            synchronous I/O will be done.
//
// Remarks  :
//
void
Socket::setAsyncMode (const bool bMode)
{
	m_bAsyncMode = bMode;
}


// Abstract : Returns the socket handle
//
// Returns  : SOCKET
// Params   :
//   -
//
// Pre      :
// Post     :
//
// Remarks  :
//
Socket::operator SOCKET (void) const
{
	return m_hFile;
}


int
Socket::getHandle (void) const
{
    return m_hFile;
}

// Abstract : Send an IPC message over the socket
//
// Returns  : int (0 on success)
// Params   :
//   pData                     Pointer to IPC structure (or sub-class)
//   pCallback                 Optional pointer to application's one-time-
//                             only callback.
//
// Pre      : The user-defined part of pData is already filled-in before
//            the application calls this routine.
// Post     : The IpcStruct data packet will be sent to the peer of
//            the socket connection.  The static packet sequence number
//            is increased by 1.
//
// Remarks  : The IpcStruct data packet will be sent across the socket
//            connection, either synchronously or asynchronously depending
//            on the I/O mode (see the SetAsyncMode function).  If the
//            mode is synchronous, then the packet will be first sent
//            over the socket before this routine returns.  In the
//            asynchronous mode, a worker thread will be made to do the
//            write and this routine returns immediately.  When the worker
//            thread completes, the application's callback routine is called.
//            If the pCallback parameter is 0, then the routine that
//            was given as parameter to RegisterCallback() will be called.
//            Otherwise, if pCallback is specified, then it will be
//            called instead.
//
int
Socket::remoteProcedure(IpcStruct* pData, Callback pCallback)
{
	// Note that the precondition states that the caller is responsible for
	// filling in the user-defined data of the pData buffer.  This actually
	// a precondition for the application that will be receiving this message.

	Callback pOldCallback;

	pData->dwSequence_ = ++m_dwSequence;
	if (pCallback)
	{
		pOldCallback = (Callback) registerCallback(pCallback);
	}

	write(pData, pData->wPacketSize_);

	if (pCallback)
	{
		registerCallback(pOldCallback);
	}
	return 0;
}


// Abstract : Read an IPC message or reply from socket
//
// Returns  : int (number of bytes read)
// Params   :
//   pData                     Pointer to IPC structure (or sub-class)
//   uMaxLength                The maximum length that can be read into
//                             the pData buffer.
//
// Pre      : The application must already have allocated the pData buffer.
//            If the input parameter uMaxLength is not zero, then this routine
//            makes sure to read no more than uMaxLength bytes into the pData
//            buffer.
// Post     : Upon completion, pData will be filled with the IPC structure
//            from the network.  This routine does not check to see if the
//            data is a valid IPC packet.  The application will need to
//            check the member variables in the IpcStruct header to
//            find out if the buffer contains an expected IPC packet.
//
// Remarks  : The application should not attempt to modify or free the
//            buffer pointed to by pData until this routine completes.
//            For a asynchronous call, completion means when the callback
//            routine is called.
//
//            This routine has no way of knowing which IPC message or reply
//            has been sent and therefore doesn't know how much to read.
//            It is possible that 2 messages are in the buffer and a
//            single read here would concatenate them into one buffer (which
//            is not what we want).  Thus, this routine first reads only the
//            header which contains the packet length.  Then the rest of the
//            packet is appended with a second read.
//
int
Socket::remoteReadData(IpcStruct* pData, UINT uMaxLength)
{
	// Warning: This routine does not work in overlapped I/O mode.  The
	//          next statement (temporarily) goes to polling mode.
	Callback pOldCallback = (Callback) registerCallback();
	UINT uLength = pData->wPacketSize_;
	if (uMaxLength != 0 && uMaxLength < uLength)
	{
		uLength = uMaxLength;
	}
	VERIFY(uLength >= sizeof(IpcStruct));

	bool bValidIPC = false;
	UINT uActual;
	while (! bValidIPC)
	{
		// Read in just the header
		uActual = read(pData, sizeof(IpcStruct));
		if (uActual == 0)		// No data, so just return.
		{
			registerCallback(pOldCallback);
			return 0;
		}
#ifdef _DEBUG 
		VERIFY(uActual == sizeof(IpcStruct));
		// OK, got the header.  Now read the rest of the packet.
		VERIFY(uLength >= pData->m_wPacketSize);
#else
		if (uActual != sizeof(IpcStruct))
		{
			continue;
		}
		if (uLength < pData->wPacketSize_)
		{
			// flush buffer
			DWORD dwBytes;
			if (::ioctl(m_hFile, FIONREAD, &dwBytes) == SOCKET_ERROR)
			{
				registerCallback(pOldCallback);
				return 0;
			}
			if (dwBytes)
			{
				char * pDiscardBuf = new char[dwBytes];
				read(pDiscardBuf, dwBytes);
				delete [] pDiscardBuf;
			}
			continue;
		}
#endif
		uLength = pData->wPacketSize_ - uActual;
		uActual = read((char *)pData + uActual, uLength);
#ifdef _DEBUG
		VERIFY(uActual == uLength);
#else
		if (uActual == uLength)
#endif
		{
			bValidIPC = true;	// Finally, declare packet as valid
		}
	}

	// Reset the user's callback to what it was
	registerCallback(pOldCallback);
	return uActual + sizeof(IpcStruct);
}


// Abstract : Send a reply to an IPC message
//
// Returns  : int (0 on success)
// Params   :
//   pData                     Pointer to IPC reply structure (or sub-class)
//   dwSequence                Option sequence number of IPC to which this
//                             reply refers.
//
// Pre      : It is "assumed" that the peer has sent a remote procedure to
//            which this application is replying.  No attempt is made to
//            enforce this condition.
// Post     : The IpcReplyStruct data packet will be sent back to the
//            peer of the socket connection.
//
// Remarks  :
//
int
Socket::remoteWriteReply(IpcReplyStruct* pData, DWORD dwSequence)
{
	// Fill-in the cookie, if caller specifies it separately
	if (dwSequence)
	{
		pData->dwSequence_ = dwSequence;
	}

	write(pData, pData->wPacketSize_);

	return 0;
}


///////////////////////////////////////////////////////////
//   STATE-DEPENDENT FUNCTIONS FOLLOW BELOW :
///////////////////////////////////////////////////////////

// Abstract : Executes the state-dependent abort
//
// Returns  : -
// Params   :
//   -
//
// Pre      :
// Post     : Pending I/O on the socket is cancelled.
//
// Remarks  :
//
void
Socket::abort(void)
{
	m_pState->abort(this);
}


// Abstract : Executes the state-dependent close
//
// Returns  : -
// Params   :
//   -
//
// Pre      :
// Post     : The socket connection is closed, if it was open.
//
// Remarks  :
//
void
Socket::close(void)
{
    if (m_hFile != INVALID_SOCKET)
        m_pState->close(this);
}


// Abstract : Executes the state-dependent create server
//
// Returns  : Socket&
// Params   :
//   wPort                     TCP/IP port number of new server socket
//   pHost                     Optional IP address of new server socket
//
// Pre      :
// Post     : If this function completes successfully, it will return
//            a reference to a newly created server socket object.
//
// Remarks  :
//
Socket&
Socket::createServer(const WORD wPort, const IPAddress* pHost)
{
	return m_pState->createServer(this, wPort, pHost);
}


// Abstract : Retrieves a socket option (state-dependent)
//
// Returns  : bool (true on success)
// Params   :
//   nOptionName               The socket option to retrieve
//   pOptionValue              Pointer to buffer where the option will be put
//   pnOptionLen               The size of pOptionValue buffer
//   nLevel                    Optional parameter specifying the socket level
//
// Pre      :
// Post     : The value of the option will be returned in pOptionValue and
//            the pnOptionLen will be set to the size of the returned value.
//
// Remarks  : This function is just a thin layer on top of the ::getsockopt()
//            routine of the Windows Sockets library.
//
bool
Socket::getSockOpt(int  nOptionName, void* pOptionValue,
					   socklen_t* pnOptionLen,  int  nLevel)
{
	return m_pState->getSockOpt(this, nOptionName, pOptionValue,
		 						      pnOptionLen, nLevel);
}


// Abstract : Executes the state-dependent listen
//
// Returns  : Pointer to a newly created, connected Socket object
// Params   :
//   nBackLog                  Maximum number of clients that can be
//                             queued waiting for a connection.
//
// Pre      :
// Post     : Upon successful completion of this routine, a new socket handle
//            is created which is connected to a client.  The original server
//            socket remains in listen mode.
//
// Remarks  :
//
Stream *
Socket::listen(const int nBacklog)
{
	SOCKET ClientSocket = m_pState->listen(this, nBacklog);

	// Construct a new client socket object
	Socket * pClient = new Socket;
	pClient->m_hFile = ClientSocket;
	pClient->m_uOpenFlags = m_uOpenFlags;
	pClient->m_bAsyncMode = m_bAsyncMode;

	if (ClientSocket == INVALID_SOCKET)
	{
		//pClient->m_pState = SSClosed::instance();
		//pClient->m_Status  = SC_FAILED;
        delete pClient;
        pClient = 0;
	}
	else
	{
		pClient->m_pState = SSConnected::instance();

		// Only AFTER the listen do we know who's calling
		socklen_t iSizeAddr = sizeof(sockaddr);
		::getpeername(ClientSocket, (sockaddr *) &pClient->m_PeerAddr, &iSizeAddr);
	}
	return pClient;
}


// Abstract : Opens a socket connection for the Socket object
//            (state-dependent)
//
// Returns  : bool (true on success)
// Params   :
//   lpszFileName              Name of socket to open (in URL format)
//   rSockAddr                 Reference to a socket address to open
//   uOpenFlags                Flags indicating how the socket should be opened
//
// Pre      : The first form of Open expects the lpszFileName parameter to
//            contain a valid URL name representing the socket to open.
//            The second form of Open expects a well-formed SocketAddr.
// Post     : Upon successful completion of this routine, either a server
//            socket or a client socket will be opened.  In the case of
//            a server socket, the application will need to call Listen in
//            order to connect the socket to a client.
//            The network address of rSockAddr is tested and depending on its
//            value:
//            - If the network address is equal to INADDR_NONE, then the socket
//              remains closed, m_Status is set to SC_FAILED, and false is
//              returned.
//            - If the network address is equal is INADDR_ANY or the uOpenFlags
//              included modeCreate then the state is changed to 
//              SSOpenedServer and an attempt is made to open a server-side
//              socket.
//            - If neither of the previous 2 conditions applies, then the state
//              is changed to SSOpenedClient and an attempt is made to open
//              a client-side socket.
//            If the state-dependent open fails then m_State is set to SC_FAILED
//            and Open returns false.
//            Otherwise (open succeded), m_PeerAddr is filled with a textual
//            representation of the peer's address, the status is set to SC_OK
//            and true is returned.
//
// Remarks  :
//
bool
Socket::open(const char* lpszFileName, UINT uOpenFlags)
{
    WORD wPort = 0;
    size_t nColon;
    std::string Name;
    std::string Host;

    if (lpszFileName != 0 && strlen(lpszFileName) != 0)
    {
        // Parse the file name into a socket address object.  Then
        // call the common state-dependent Open().
        Name = lpszFileName;
        size_t nBegin = 0;
        nColon = Name.find_last_of(':');

        nBegin = Name.find("//");
        if (nBegin == std::string::npos)
            nBegin = 0;
        else
            nBegin += 2;

        if (nColon == std::string::npos)
        {
            Host = Name.substr(nBegin);
            if (Host.length())
            {	// Error: Hostname specified but no port number.  This would
                //        indicate a desire to connect to ANY port as client.
                m_Status = SC_FAILED;
                return false;
            }
        }
        else
        {
            size_t fColon = Name.substr(nBegin).find_first_of(':');
            if (nColon == fColon)
            {
                if (nColon == 0)	// Line begins with colon
                {
                    Host = "";
                }
                else
                {
                    Host  = Name.substr(nBegin, nColon - nBegin);
                }
                wPort = atoi(Name.substr(nColon + 1).c_str());
            }
            else	// multiple colons, maybe IPv6
            {
                nColon = std::string::npos;
                Host  = Name.substr(nBegin);
            }
        }
    }

    if (wPort == 0)
    {
        if (Name.substr(0,5) == "http:")
            wPort = 80;
        else if (Name.substr(0,6) == "https:")
            wPort = 443;
    }
        
    if (wPort == 0 && nColon != std::string::npos)
    {
        SocketAddr SockAddr(Host.c_str(), Name.substr(nColon + 1).c_str());
        return open(SockAddr, uOpenFlags);
    }
    else
    {
        SocketAddr SockAddr(Host.c_str(), wPort);
        return open(SockAddr, uOpenFlags);
    }
}


bool
Socket::open(SocketAddr& rSockAddr, UINT uOpenFlags)
{
	if (rSockAddr.m_pProtocol != 0 && strcasecmp(rSockAddr.m_pProtocol, "udp") == 0)
		m_nProtocol = SOCK_DGRAM;
	else
		m_nProtocol = SOCK_STREAM;

	if (rSockAddr.netAddress() == INADDR_NONE && m_nProtocol == SOCK_STREAM)
	{
		m_Status = SC_FAILED;
		return false;
	}
	if (rSockAddr.netAddress() == INADDR_ANY || (uOpenFlags & modeCreate))
		m_pState = SSOpenedServer::instance();
    else if (rSockAddr.portNumber() == 443)
    {
        std::cout << "SSL connection" << std::endl;
        m_pState = SSOpenedClientTLS::instance();
    }
	else
		m_pState = SSOpenedClient::instance();

	if (! m_pState->open(this, rSockAddr, uOpenFlags))
	{
		m_Status = SC_FAILED;
		return false;
	}

	// Initialize member in case getsockname fails
	m_PeerAddr.sin_family = rSockAddr.sin_family;
	m_PeerAddr.sin_port = rSockAddr.sin_port;
	m_PeerAddr.sin_addr.s_addr = rSockAddr.sin_addr.s_addr;

	// Get the name of our peer.  For a server socket this will
	// probably be a broadcast address and not really a peer.  But
	// go ahead and get it in case the LPCSTR operator is called.
	socklen_t iSizeAddr = sizeof(sockaddr);
	::getsockname(m_hFile, (sockaddr *) &m_PeerAddr, &iSizeAddr);

	if (m_nProtocol == SOCK_DGRAM)
	{
		m_PeerAddr.sin_port = rSockAddr.sin_port;
		m_PeerAddr.sin_addr.s_addr = INADDR_BROADCAST;
	}
	else
	{
		rSockAddr.sin_port = m_PeerAddr.sin_port;
	}

	m_Status = SC_OK;
	return true;
}


// Abstract : Executes the state-dependent read
//
// Returns  : UINT (actual number of bytes read)
// Params   :
//   pBuf                      Buffer that will be filled
//   uCount                    Size of buffer
//
// Pre      :
// Post     : This routine will fill pBuf with data that is available
//            from the socket.  It will read no more than the maximum
//            given by the uCount parameter.  This routine returns the
//            number of bytes actually read.
//
// Remarks  :
//
UINT
Socket::read(void* pBuf, UINT uCount)
{
	if (uCount == 0)
		return 0;		// In that case, we are done quickly.

	return m_pState->read(this, pBuf, uCount);
}

UINT
Socket::read(std::string& str, int delimiter)
{
    char buf[32];
    int ret = 0;
    int sz;

    str.clear();
    while ((sz = read(buf, 1)) > 0)
    {
        ret += sz;
        for (int i = 0; i < sz; i++)
        {
            str.append(1, buf[i]);
            if (buf[i] == delimiter)
                return ret;
        }
    }
    return ret;
}

// Abstract : Executes the state-dependent reader thread
//
// Returns  : THRTYPE
// Params   :
//   pThis                     Pointer to the socket object
//
// Pre      : The buffer pointed to by pIOP->m_pBuf must not be re-used or
//            freed by the application until the user's callback routine
//            has been called, indicating that the overlapped I/O is
//            complete.
// Post     :
//
// Remarks  : This (protected) routine is not called by user applications.
//            It is only called by the SocketState friend class, or 
//            one of its sub-classes.
//
THRTYPE WINAPI Socket::readerThread(LPVOID _pIOP)
{
	IOPARAMS* pIOP = (IOPARAMS*) _pIOP;
#ifdef _DEBUG
	if (m_pLastBuffer == pIOP->m_pBuf)
	{
//		TRACE("Buffer in use for overlapped I/O\n");
		VERIFY(0);
	}
	m_pLastBuffer = pIOP->m_pBuf;
#endif

    SocketState* pState = pIOP->m_pSocket->m_pState;
	DWORD  dwReturn;
	dwReturn = pState->readerThread(pIOP);

#ifdef _DEBUG
	m_pLastBuffer = 0;
#endif
	delete pIOP;

#ifdef THRTYPE_NOCAST
	return dwReturn;
#else
	return reinterpret_cast<THRTYPE>(dwReturn);
#endif
}


// Abstract : Sets a socket option (state-dependent)
//
// Returns  : int (TRUE on success)
// Params   :
//   nOptionName               The socket option to retrieve
//   pOptionValue              Pointer to buffer where the option will be put
//   nOptionLen                The size of pOptionValue buffer
//   nLevel                    Optional parameter specifying the socket level
//
// Pre      :
// Post     : The value of the option will be changed to pOptionValue.
//
// Remarks  : This function is just a thin layer on top of the ::setsockopt()
//            routine of the Windows Sockets library.
//
int
Socket::setSockOpt(int  nOptionName, const void* pOptionValue,
					   int  nOptionLen, int nLevel)
{
	return m_pState->setSockOpt(this, nOptionName, pOptionValue,
		 						      nOptionLen,  nLevel);
}


// Abstract : Executes the state-dependent write
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
// Remarks  :
//
void
Socket::write(const void* pBuf, UINT uCount)
{
	m_pState->write(this, pBuf, uCount);
}


void
Socket::write(const std::string& str)
{
    m_pState->write(this, str.c_str(), str.size());
}


// Abstract : Executes the state-dependent writer thread
//
// Returns  : THRTYPE
// Params   :
//   pThis                     Pointer to the socket object
//
// Pre      :
// Post     :
//
// Remarks  : This (protected) routine is not called by user applications.
//            It is only called by the SocketState friend class, or 
//            one of its sub-classes.
//
THRTYPE WINAPI Socket::writerThread(LPVOID _pIOP)
{
	IOPARAMS* pIOP = (IOPARAMS*) _pIOP;
#ifdef _DEBUG
	if (m_pLastBuffer == pIOP->m_pBuf)
	{
//		TRACE("Buffer in use for overlapped I/O\n");
		VERIFY(0);
	}
	m_pLastBuffer = pIOP->m_pBuf;
#endif

    SocketState* pState = pIOP->m_pSocket->m_pState;
	DWORD  dwReturn;
	dwReturn = pState->writerThread(pIOP);
#ifdef _DEBUG
	m_pLastBuffer = 0;
#endif

	delete pIOP;

#ifdef THRTYPE_NOCAST
	return dwReturn;
#else
	return reinterpret_cast<THRTYPE>(dwReturn);
#endif
}


// Abstract : Returns a static, textual representation of an address
//
// Returns  : char*
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
Socket::operator char* (void) const
{
	static char szHostName[124];
	UINT dwAddress = m_PeerAddr.sin_addr.s_addr;
    char tmpName[100];
//	struct hostent* pHostEntry;
//	pHostEntry = ::gethostbyaddr((char *)&dwAddress, sizeof(dwAddress), AF_INET);
//	if (pHostEntry != 0)
    struct addrinfo* pAddrInfo = 0;
    if (::getnameinfo((const sockaddr *) &m_PeerAddr, sizeof(m_PeerAddr),
                      tmpName, sizeof(szHostName), 0, 0, 0) == 0)
	{
		sprintf(szHostName, "%s:%hu", tmpName, ntohs(m_PeerAddr.sin_port));
        ::freeaddrinfo(pAddrInfo);
	}
    else
    {
        // Reverse DNS failed, use TCP/IP dot notation
        sprintf(szHostName,"%0d.%0d.%0d.%0d:%hu",
                dwAddress & 0xff, (dwAddress >> 8) & 0xff,
                (dwAddress >> 16) & 0xff, (dwAddress >> 24) & 0xff,
                ntohs(m_PeerAddr.sin_port));
    }
	return szHostName;
}


// Abstract : Goes to the next specified state.
//
// Returns  : -
// Params   :
//   pState                    Pointer to the next state
//
// Pre      :
// Post     : The Socket object's state will be changed.
//
// Remarks  : This routine is part of the State machine mechanics
//
void Socket::changeState(SocketState* pState)
{
	m_pState = pState;
}


//
// END OF FILE
//
