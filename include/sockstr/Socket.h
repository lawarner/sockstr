/*
   Copyright (C) 2012, 2013
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

#ifndef _SOCKET_H_INCLUDED_
#define _SOCKET_H_INCLUDED_
//
// INCLUDE FILES
//
#include <sockstr/sstypes.h>
#ifdef TARGET_WINDOWS
#include <WinSock2.h>
#endif
#include <string>
#include <sockstr/SocketAddr.h>
#include <sockstr/Stream.h>


//
// FORWARD CLASS DECLARATIONS
//
struct ssl_st;
struct ssl_ctx_st;
typedef struct ssl_st SSL;
typedef struct ssl_ctx_st SSL_CTX;


namespace sockstr
{
/**
  @class Socket
  This class encapsulates the functionality for one end of a socket connection.
      The socket can be either a client-side or server-side connection, depending
	  on how the open() routine is invoked.
*/  
//
// Members    :
//   Data
//
//     m_PeerName    Saved socket address of our peer (other end of connection)
//     m_uOpenFlags  I/O modes of how the socket is opened
//     m_bAsyncMode  Indicator if I/O is asynchronously performed
//
//   ----------------------------------------------------------------
//

//
// MACRO DEFINITIONS
//
#ifndef DllExport
#define DllExport
#endif

/** Level to use for our custom sockopt settings. */
#define SOL_SOCKSTR  300
#define SO_SOCKSTR_SSL_KEY      1
#define SO_SOCKSTR_SSL_KEYFILE  2
#define SO_SOCKSTR_SSL_PASSWORD 3
#define SO_SOCKSTR_SSL_CAFILE   4
#define SO_SOCKSTR_SSL_CAPATH   5

//
// TYPE DEFINITIONS
//
// This structure is used to pass parameters to worker threads.
struct IOPARAMS
{
	Socket* m_pSocket;
	void*      m_pBuf;
	UINT        m_uCount;
	Callback m_pCallback;
};


//
// FORWARD CLASS DECLARATIONS
//
class SocketState;
class ReadThreadHandler;
class WriteThreadHandler;


//
// CLASS DECLARATION
/*!
  A Stream using TCP/IP sockets as transport.
*/
class DllExport Socket : public Stream
{
public:
	//!  Constructs a Socket object.
	Socket(void);
	/*!
	 *   Constructs a Socket object.
     *   @param lpszFileName Hostname or dotted IP address
     *   @param uOpenFlags   Mode to open socket
     */
	Socket(const char* lpszFileName, UINT uOpenFlags);
	/*!
	 *   Constructs a Socket object.
	 *   @param rSockAddr    Use the SocketAddr to establish a connection
	 *   @param uOpenFlags   Mode to open socket
     */
	Socket(SocketAddr& rSockAddr, UINT uOpenFlags);
	/*!
	 *   Destruct a Socket object and close socket
     */
	virtual ~Socket(void);
	/*!
	     Return socket handle of the connection.
	*/
	operator SOCKET (void) const;

    int getHandle (void) const;

	// State-dependent functions

	//!  Cancel any pending I/O operations on stream (state-dependent).
	virtual void  abort		      (void);

	//!  Close the socket (state-dependent).
	virtual void  close           (void);

	//!  Create a new server socket from current connection
    //!  @param wPort Port number
    //!  @param pHost IPAddress of host
	Socket&       createServer    (const WORD wPort, 
								   const IPAddress* pHost = 0);
	//!   Query socket options
	bool          getSockOpt      (int  nOptionName, void* pOptionValue,
					 			   socklen_t* pnOptionLen, int nLevel = SOL_SOCKET);
	//! Listen for incoming connection on server socket (state-dependent)
	virtual Stream *
				  listen          (const int nBacklog = 4);
	//! Open a client or server socket (state-dependent)
	virtual bool  open            (const char* lpszFileName, UINT uOpenFlags);
	virtual bool  open			  (SocketAddr& rSockAddr, UINT uOpenFlags);
	//! Read from socket (state-dependent)
	virtual UINT  read            (void* pBuf, UINT uCount);
	//!  Read a string from the stream (state-dependent).
    virtual UINT  read            (std::string& str, int delimiter='\n');
	//! Send an IPC message over the socket (state-dependent)
	virtual int   remoteProcedure (IpcStruct* pData, Callback pCallback = 0);
	//! Read an IPC message or reply from socket (state-dependent)
	virtual int   remoteReadData  (IpcStruct* pData, UINT uMaxLength = 0);
	//!     RemoteWriteReply Send a reply to an IPC message (state-dependent)
	virtual int   remoteWriteReply(IpcReplyStruct* pData, DWORD dwSequence = 0);
	//!   Asynchronous I/O mode on or off.
	virtual void  setAsyncMode    (const bool bMode);
	//!   Set socket options.
	int           setSockOpt      (int  nOptionName, const void* pOptionValue,
					 			   int  nOptionLen,  int nLevel = SOL_SOCKET);
	//! Write to socket (state-dependent)
	virtual void  write           (const void* pBuf, UINT uCount);
	//!  Write a string to the stream (state-dependent).
    virtual void  write           (const std::string& str);

	/** Returns a static, textual representation of an address
     *  (i.e., "host.acme.com:1074").  The value returned is an internal
     *  static and is thus not thread safe.
     */
	virtual operator const char* (void) const;
	//!   Assignment operator
	Socket& operator=(const Socket& rSource);

protected:
    Stream* listenIntern(Socket* pClient, const int nBacklog);

public:
    /** Open flags. */
    static const int modeCreate;		//!< Create a server socket
    static const int modeAsyncSocket;	//!< Socket will be asynchronous by default
    static const int modeRead;			//!< Socket can only be read from
    static const int modeWrite;			//!< Socket can only be written to
    static const int modeReadWrite;		//!< Socket can be read from and written to

protected:
	IPAddress   m_IpAddr;
	sockaddr_in m_PeerAddr;
	UINT		m_uOpenFlags;
	bool        m_bAsyncMode;
	UINT		m_nProtocol;
#if USE_OPENSSL
    SSL*        m_pSsl;
    SSL_CTX*    m_pSslCtx;
#endif

private:
	// Counter for IPC messages (generates magic cookies)
	static DWORD  m_dwSequence;

private:
	/// Do initializations that are common to all constructors.
	void initialize(void);

	// Disable copy constructor
	Socket(const Socket&);

	// State machine
	friend class SocketState;
	friend class SSClosed;
	friend class SSConnected;
	friend class SSListening;
	friend class SSOpenedServer;	// this should be called SSOpeningServer
	friend class SSOpenedClient;
	friend class SSReading;
	friend class SSWriting;
#if USE_OPENSSL
	friend class SSOpenedClientTLS;
	friend class SSConnectedTLS;
#endif
    friend class ReadThreadHandler;
    friend class WriteThreadHandler;

	// Goes to the next specified state.
	void changeState(SocketState* pState);
	SocketState* m_pState;		// Pointer to current state
};

}
#endif
