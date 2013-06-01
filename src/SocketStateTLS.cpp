/*
   Copyright (C) 2013
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
// File       : SocketStateTLS.cpp
//
// Class      : subclases of SocketState
//
// Description: This file contains states that involve TLS handling.
//
// Decisions  : 
//

//
// INCLUDE FILES
//
#include "config.h"
#if USE_OPENSSL

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

#include <openssl/ssl.h>

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
static int sockstr_password_cb(char *buf, int size, int rwflag, void* userdata);

//
// DATA DEFINITIONS
//

// Initialize static members
SocketState* SSOpenedClientTLS::m_pInstance = 0;
SocketState* SSConnectedTLS::m_pInstance = 0;

//
// CLASS MEMBER FUNCTION DEFINITIONS
//

SSOpenedClientTLS::SSOpenedClientTLS()
    : m_key("sockstr.pem")
    , m_password("password")
    , m_cafile("")
    , m_capath("/etc/ssl/cert")
{

}

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
    if (!SSL_CTX_use_certificate_chain_file(ctx, m_key.c_str()))
    {
        SSL_CTX_free(ctx);
        return false;
    }
    SSL_CTX_set_default_passwd_cb(ctx, sockstr_password_cb);
    SSL_CTX_set_default_passwd_cb_userdata(ctx, (void *) m_password.c_str());
    // Usually only one of either cafile or capath would be set, but not both.
    if (!SSL_CTX_use_PrivateKey_file(ctx, m_key.c_str(), SSL_FILETYPE_PEM) ||
        !(SSL_CTX_load_verify_locations(ctx,
                                        m_cafile.empty() ? 0 : m_cafile.c_str(),
                                        m_capath.empty() ? 0 : m_capath.c_str())))
    {
        SSL_CTX_free(ctx);
        return false;
    }
#if (OPENSSL_VERSION_NUMBER < 0x00905100L)
    SSL_CTX_set_verify_depth(ctx, 1);
#endif

//    if (ciphers){
//      SSL_CTX_set_cipher_list(ctx, ciphers);
//    }

	// If broadcast (connectionless) then m_nProtocol is SOCK_DGRAM,
	//  else it is SOCK_STREAM. DGRAM is not supported for TLS.
	pSocket->m_hFile = ::socket(AF_INET, pSocket->m_nProtocol, 0);
	if (pSocket->m_hFile == INVALID_SOCKET)
    {
        SSL_CTX_free(ctx);
		return false;
    }

	if (pSocket->m_nProtocol == SOCK_STREAM)
	{
		if (::connect(pSocket->m_hFile, (sockaddr *)rSockAddr, sizeof(sockaddr))
			== SOCKET_ERROR)
        {
            SSL_CTX_free(ctx);
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
	}
	else
	{
		// Cannot do UDP on a secure socket
        SSL_CTX_free(ctx);
        close(pSocket);
        return false;
	}

    // SSL context has been initialized, socket is opened and connected.
    // Now, connect and SSL socket
    SSL* ssl = SSL_new(ctx);
    BIO* sbio = BIO_new_socket(pSocket->m_hFile, BIO_NOCLOSE);
    SSL_set_bio(ssl, sbio, sbio);

    if (SSL_connect(ssl) <= 0)
	{
        SSL_CTX_free(ctx);
        close(pSocket);
        return false;
	}

    //TODO check cert here

	// Open was successful -- next state
    pSocket->m_pSslCtx = ctx;
    pSocket->m_pSsl    = ssl;

	changeState(pSocket, SSConnectedTLS::instance());
	return true;
}


static int sockstr_password_cb(char *buf, int size, int rwflag, void* userdata)
{
    strncpy(buf, static_cast<const char *>(userdata), size);
    buf[size - 1] = '\0';
    return strlen(buf);
}


bool
SSOpenedClientTLS::getSockOpt(Socket* pSocket,
                              int  nOptionName, void* pOptionValue,
                              socklen_t* nOptionLen,  int   nLevel)
{
    //TODO decide if I want to implement returning key, keyfile or password
    //     due to certain security issues.

    return SocketState::getSockOpt(pSocket, nOptionName, pOptionValue,
                                   nOptionLen,nLevel);
}

bool
SSOpenedClientTLS::setSockOpt(Socket* pSocket,
                              int nOptionName, const void* pOptionValue,
                              int nOptionLen, int nLevel)
{
    if (nLevel == SOL_SOCKSTR)
    {
        bool ret = true;
        switch (nOptionName)
        {
        case SO_SOCKSTR_SSL_KEY:
            m_key.assign(static_cast<const char*>(pOptionValue), nOptionLen);
            break;
        case SO_SOCKSTR_SSL_KEYFILE:
            m_key.assign(static_cast<const char*>(pOptionValue), nOptionLen);
            break;
        case SO_SOCKSTR_SSL_PASSWORD:
            m_password.assign(static_cast<const char*>(pOptionValue), nOptionLen);
            break;
        case SO_SOCKSTR_SSL_CAFILE:
            m_cafile.assign(static_cast<const char*>(pOptionValue), nOptionLen);
            break;
        case SO_SOCKSTR_SSL_CAPATH:
            m_capath.assign(static_cast<const char*>(pOptionValue), nOptionLen);
            break;
        default:
            // Error
            ret = false;
        }
        return ret;
    }

    return SocketState::setSockOpt(pSocket, nOptionName, pOptionValue,
                                   nOptionLen, nLevel);
}


//  Returns the one (and only) instance of this object
SocketState*
SSConnectedTLS::instance(void)
{
	if (m_pInstance == 0)
	{
		m_pInstance = new SSConnectedTLS;
	}
	return m_pInstance;
}

//  Closes the socket connection
void
SSConnectedTLS::close(Socket* pSocket)
{
    // close down the SSL
    SSL_CTX_free(pSocket->m_pSslCtx);

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
//            a TCP/IP socket.  It does not support UDP.
int
SSConnectedTLS::readSocket(Socket* pSocket, void* pBuf, UINT uCount)
{
	int iResult = 0;

    iResult = SSL_read(pSocket->m_pSsl, pBuf, uCount);

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
        iResult = SSL_write(pSocket->m_pSsl, (const char *)pBuf, uCount);

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
    iResult = SSL_write(pIOP->m_pSocket->m_pSsl, (const char *)pIOP->m_pBuf,
                        pIOP->m_uCount);

	if (iResult <= 0)
		return 1;			// Thread exit code 1 == failure

	pIOP->m_pCallback(iResult, pIOP->m_pBuf);

	return 0;
}

#endif // USE_OPENSSL
//
// END OF FILE
//
