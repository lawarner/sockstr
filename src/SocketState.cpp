/*
   Copyright (C) 2012, 2023
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
#include <thread>

#include <sockstr/Socket.h>
#include <sockstr/SocketState.h>

//
// FORWARD FUNCTION DECLARATIONS
//

//
// DATA DEFINITIONS
//

namespace sockstr {
// Initialize static members
SocketState* SSClosed::m_pInstance = nullptr;
SocketState* SSOpenedServer::m_pInstance = nullptr;
SocketState* SSOpenedClient::m_pInstance = nullptr;
SocketState* SSListening::m_pInstance = nullptr;
SocketState* SSConnected::m_pInstance = nullptr;
#ifdef _DEBUG
void* SocketState::m_pLastBuffer = 0;
#endif

namespace {

DWORD status_ = 0;   // TODO use a member of Socket

}  // namespace

void SocketState::read_thread_handler(IOPARAMS* pIOP) {
    SocketState* pState = pIOP->m_pSocket->m_pState;
    DWORD  dwReturn;
    dwReturn = pState->readerThread(pIOP);

    delete pIOP;
    status_ = dwReturn;
}


void SocketState::write_thread_handler(IOPARAMS* pIOP) {
    SocketState* pState = pIOP->m_pSocket->m_pState;
    DWORD  dwReturn;
    dwReturn = pState->writerThread(pIOP);

    delete pIOP;
    status_ = dwReturn;
}

//
// CLASS MEMBER FUNCTION DEFINITIONS
//

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
void SocketState::abort(Socket* pSocket) {
    // Not interested in return value, just always try to abort blocking I/O
    if (pSocket->m_hFile) {
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
/* Does this add to DOS line endings? */

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
DWORD SocketState::readerThread(IOPARAMS* /*pIOP*/) {
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
bool SocketState::setSockOpt(Socket* pSocket,
                         int nOptionName, const void* pOptionValue,
                         int nOptionLen, int nLevel) {
    if (pSocket->getHandle() == INVALID_SOCKET) {
        return false;
    }
    if (::setsockopt(pSocket->getHandle(), nLevel, nOptionName, (char *)pOptionValue,
                     nOptionLen) == SOCKET_ERROR) {
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
void SocketState::write(Socket* /*pSocket*/,
                        const void* /*pBuf*/, UINT /*uCount*/) {
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
DWORD SocketState::writerThread(IOPARAMS* /*pIOP*/) {
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
void SocketState::changeState(Socket* pSocket, SocketState* pState) {
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
IOPARAMS* SocketState::createIOParams(Socket* pSocket, void* pBuf, UINT uCount,
                                      Callback pCallback) {
    IOPARAMS* pIO = new IOPARAMS;
    pIO->m_pSocket   = pSocket;
    pIO->m_pBuf      = pBuf;
    pIO->m_uCount    = uCount;
    pIO->m_pCallback = pCallback;
    return pIO;
}

IOPARAMS* SocketState::createIOParams(Socket* pSocket, const void* pBuf, UINT uCount,
                                      Callback pCallback) {
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

SocketState* SSClosed::instance() {
    if (m_pInstance == nullptr) {
        m_pInstance = new SSClosed;
    }
    return m_pInstance;
}

SocketState* SSOpenedServer::instance() {
    if (m_pInstance == nullptr) {
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
bool SSOpenedServer::open(Socket* pSocket, SocketAddr& rSockAddr, UINT uOpenFlags) {
    // Assume failure
    changeState(pSocket, SSClosed::instance());

    // Save the "file" open modes in object.
    // Save the non-standard modeAsyncSocket separately.
    pSocket->m_uOpenFlags =  uOpenFlags & ~Socket::modeAsyncSocket;
    pSocket->m_bAsyncMode = (uOpenFlags & Socket::modeAsyncSocket) ? true : false;

    // If broadcast (connectionless) then m_nProtocol is SOCK_DGRAM,
    //  else it is SOCK_STREAM.
    pSocket->m_hFile = ::socket(AF_INET6, pSocket->m_nProtocol, 0);
    if (pSocket->m_hFile == INVALID_SOCKET) {
        return false;
    }
    // TODO: Maybe skip the sockopt if portnum = 0?
#ifdef TARGET_WINDOWS
    bool bSockOpt = true;
#else
    int bSockOpt = 1;
#endif
    ::setsockopt(pSocket->m_hFile, SOL_SOCKET, SO_REUSEADDR,
                 (char *)&bSockOpt, sizeof(bSockOpt));

    if (pSocket->m_nProtocol == SOCK_STREAM) {
        ::setsockopt(pSocket->m_hFile, SOL_SOCKET, SO_KEEPALIVE,
                     (char *)&bSockOpt, sizeof(bSockOpt));
    }

    sockaddr_storage sa;
    socklen_t len;
    if (!rSockAddr.getSockAddr(sa, len)) {
        close(pSocket);
        return false;
    }
    if (::bind(pSocket->m_hFile, (const sockaddr*)&sa, len) == SOCKET_ERROR) {
        close(pSocket);
        return false;
    }

    if (pSocket->m_nProtocol == SOCK_STREAM) {
        if (::listen(pSocket->m_hFile, SOMAXCONN) == SOCKET_ERROR) {
            close(pSocket);
            return false;
        }
        // Open was successful -- next state
        changeState(pSocket, SSListening::instance());
    } else {    // SOCK_DGRAM
#ifdef TARGET_WINDOWS
        bSockOpt = true;
#else
        bSockOpt = 1;
#endif
        ::setsockopt(pSocket->m_hFile, SOL_SOCKET, SO_BROADCAST,
                     (char *)&bSockOpt, sizeof(bSockOpt));

        //sa.sin_addr.s_addr = INADDR_BROADCAST;

        changeState(pSocket, SSConnected::instance());
    }
    return true;
}

SocketState* SSOpenedClient::instance() {
    if (m_pInstance == nullptr) {
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
bool SSOpenedClient::open(Socket* pSocket, SocketAddr& rSockAddr, UINT uOpenFlags) {
    // Assume failure
    changeState(pSocket, SSClosed::instance());

    // Save the "file" open modes in object.  Save the non-standard
    // modeAsyncSocket separately.
    pSocket->m_uOpenFlags = uOpenFlags & ~Socket::modeAsyncSocket;
    pSocket->m_bAsyncMode = (uOpenFlags & Socket::modeAsyncSocket) ? true : false;

    // If broadcast (connectionless) then m_nProtocol is SOCK_DGRAM,
    // else it is SOCK_STREAM.
    pSocket->m_hFile = ::socket(pSocket->m_nFamily, pSocket->m_nProtocol, 0);
    if (pSocket->m_hFile == INVALID_SOCKET) {
        return false;
    }
    sockaddr_storage sa;
    socklen_t len;
    if (!rSockAddr.getSockAddr(sa, len)) {
        return false;
    }
    if (pSocket->m_nProtocol == SOCK_STREAM) {
        if (::connect(pSocket->m_hFile, (const sockaddr*)&sa, len) == SOCKET_ERROR) {
            return false;
        }
#ifdef TARGET_WINDOWS
        bool bSockOpt = true;
#else
        int bSockOpt = 1;
#endif
        ::setsockopt(pSocket->m_hFile, SOL_SOCKET, SO_KEEPALIVE,
                     (char *)&bSockOpt, sizeof(bSockOpt));
    } else {
        // For UDP, client needs to bind with port 0.
        sockaddr_in BindAddr;
        BindAddr.sin_family      = AF_INET;
        BindAddr.sin_addr.s_addr = INADDR_ANY;
        BindAddr.sin_port        = 0;

        if (::bind(pSocket->m_hFile, (sockaddr *)&BindAddr, sizeof(sockaddr_in))
            == SOCKET_ERROR)
            return false;

#ifdef TARGET_WINDOWS
        bool bSockOpt = true;
#else
        int bSockOpt = 1;
#endif
        ::setsockopt(pSocket->m_hFile, SOL_SOCKET, SO_BROADCAST,
                     (char *)&bSockOpt, sizeof(bSockOpt));

        //rSockAddr.sin_addr.s_addr = INADDR_BROADCAST;
    }

    // Open was successful -- next state
    changeState(pSocket, SSConnected::instance());
    return true;
}


SocketState* SSListening::instance() {
    if (m_pInstance == nullptr) {
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
SOCKET SSListening::listen(Socket* pSocket, const int /*nBacklog*/) {
    SOCKET hSock;
    if ((hSock = ::accept(pSocket->m_hFile, 0, 0)) == INVALID_SOCKET) {
        return INVALID_SOCKET;
    }
    return hSock;
}

SocketState* SSConnected::instance() {
    if (m_pInstance == nullptr) {
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
UINT SSConnected::read(Socket* pSocket, void* pBuf, UINT uCount) {
    int iResult = 0;

    // Error: this socket is write-only
    VERIFY(!(pSocket->m_uOpenFlags & Socket::modeWrite));
    // Error: trying to read 0 bytes
    VERIFY(uCount != 0);

    if (! pSocket->m_bAsyncMode) {
        // Synchronous mode -- do a blocking read on socket
        iResult = readSocket(pSocket, pBuf, uCount);
        if (iResult == 0 || iResult == SOCKET_ERROR) {
            pSocket->m_Status = SC_NODATA;
            pSocket->setstate(std::ios::eofbit);
            return 0;
        } else {
            pSocket->clear(pSocket->rdstate() & ~std::ios::eofbit);
        }
    } else {
        // Asynchronous mode -- if data is available on socket then read
        // it.  Otherwise, set up a reader thread to wait for data.
        DWORD dwBytes;
        if (IOCTLSOCK(pSocket->m_hFile, FIONREAD, &dwBytes) == SOCKET_ERROR) {
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
        if (dwBytes /*&& ! WSAIsBlocking()*/) {
            // This much (dwBytes) can be read without blocking
            iResult = readSocket(pSocket, pBuf, std::min((UINT)dwBytes, uCount));
            if (iResult == 0 || iResult == SOCKET_ERROR) {
                pSocket->m_Status = SC_NODATA;
                pSocket->setstate(iResult == SOCKET_ERROR 
                                  ? std::ios::badbit : std::ios::eofbit);
                return 0;
            }
        } else {
            if (pSocket->m_pDefCallback == nullptr) {
                pSocket->m_Status = SC_NODATA;
                pSocket->setstate(std::ios::eofbit);
                return 0;
            }

            auto readThreadHandler = std::thread(&SocketState::read_thread_handler, this,
                                                 createIOParams(pSocket, pBuf, uCount,
                                                                pSocket->m_pDefCallback));
            readThreadHandler.detach();
        }
    }

    if (iResult >= 0) {
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
int SSConnected::readSocket(Socket* pSocket, void* pBuf, UINT uCount) {
    int iResult = 0;

    if (pSocket->m_nProtocol == SOCK_DGRAM) {
        socklen_t iSizeFrom = sizeof(sockaddr_in);
        iResult = ::recvfrom(pSocket->m_hFile, (char *)pBuf, uCount,
                             0, (sockaddr *) &pSocket->m_PeerAddr, &iSizeFrom);
    } else {
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
DWORD SSConnected::readerThread(IOPARAMS* pIOP) {
    int iResult;

    iResult = readSocket(pIOP->m_pSocket, pIOP->m_pBuf, pIOP->m_uCount);
    if (iResult == 0 || iResult == SOCKET_ERROR) {
        return 1;			// Thread exit code 1 == failure
    }
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
void SSConnected::write(Socket* pSocket, const void* pBuf, UINT uCount) {
    // Error: this socket is read-only
    VERIFY(pSocket->m_uOpenFlags & (Socket::modeReadWrite | Socket::modeWrite));

    if (! (pSocket->m_bAsyncMode && pSocket->m_pDefCallback != nullptr)) {
        int iResult;
        // Synchronous mode -- do a blocking write on socket
        if (pSocket->m_nProtocol == SOCK_DGRAM) {
            // Note that s_addr could have been overwritten by the call to recvfrom
            // pSocket->m_PeerAddr.sin_addr.s_addr = INADDR_BROADCAST;
            iResult = ::sendto(pSocket->m_hFile, (const char *)pBuf, uCount, 0,
                               (sockaddr *)&pSocket->m_PeerAddr, sizeof(pSocket->m_PeerAddr));
        } else {
            iResult = ::send(pSocket->m_hFile, (const char *)pBuf, uCount, 0);
        }
        if (iResult == SOCKET_ERROR) {
            pSocket->m_Status = SC_FAILED;
            pSocket->setstate(std::ios::failbit);
        } else {
            pSocket->clear(pSocket->rdstate() & ~std::ios::failbit);
        }
    } else {
        auto writeThreadHandler = std::thread(&SocketState::write_thread_handler, this,
                                              createIOParams(pSocket, pBuf, uCount,
                                                             pSocket->m_pDefCallback));
        writeThreadHandler.detach();
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
DWORD SSConnected::writerThread(IOPARAMS* pIOP) {
    int iResult;
    if (pIOP->m_pSocket->m_nProtocol == SOCK_DGRAM) {
        // Note that s_addr could have been overwritten by the call to recvfrom
        //pIOP->m_pSocket->m_PeerAddr.sin_addr.s_addr = INADDR_BROADCAST;
        iResult = ::sendto(pIOP->m_pSocket->m_hFile, (const char *)pIOP->m_pBuf,
                           pIOP->m_uCount, 0,
                           (sockaddr *)&pIOP->m_pSocket->m_PeerAddr,
                           sizeof(pIOP->m_pSocket->m_PeerAddr));
    } else {
        iResult = ::send(pIOP->m_pSocket->m_hFile, (const char *)pIOP->m_pBuf,
                         pIOP->m_uCount, 0);
    }
    if (iResult == SOCKET_ERROR) {
        return 1;			// Thread exit code 1 == failure
    }
    pIOP->m_pCallback(iResult, pIOP->m_pBuf);

    return 0;
}

}  // namespace sockstr
