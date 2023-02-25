/*
   Copyright (C) 2012 - 2023
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

#ifdef TARGET_WINDOWS
#include <WinSock2.h>
#endif
#include <sockstr/SocketAddr.h>
#include <sockstr/Stream.h>
#include <string>

//
// FORWARD CLASS DECLARATIONS
//
struct ssl_st;
struct ssl_ctx_st;
typedef struct ssl_st SSL;
typedef struct ssl_ctx_st SSL_CTX;


namespace sockstr {
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
struct IOPARAMS {
    Socket* m_pSocket;
    void*   m_pBuf;
    UINT    m_uCount;
    Callback m_pCallback;
};

// Forward references
class SocketState;

/**
 *  A Stream using TCP/IP or UDP sockets as transport.
 */
class DllExport Socket : public Stream {
public:
    //!  Constructs a Socket object.
    Socket();
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
    virtual ~Socket();
    /**
     * Return socket handle of the connection.
     */
    operator SOCKET() const;

    int getHandle() const;

    // State-dependent functions

    //!  Cancel any pending I/O operations on stream (state-dependent).
    virtual void abort();

    //!  Close the socket (state-dependent).
    virtual void close();

    //!   Query socket options
    bool getSockOpt(int nOptionName, void* pOptionValue,
                    socklen_t* pnOptionLen, int nLevel = SOL_SOCKET);
    //! Listen for incoming connection on server socket (state-dependent)
    virtual Stream * listen(const int nBacklog = 4);
    //! Open a client or server socket (state-dependent)
    virtual bool open(const char* lpszFileName, UINT uOpenFlags);
    virtual bool open(SocketAddr& rSockAddr, UINT uOpenFlags);
    //! Read from socket (state-dependent)
    virtual UINT read(void* pBuf, UINT uCount);
    //!  Read a string from the socket (state-dependent).
    virtual UINT read(std::string& str, int delimiter='\n');
    //!  Read a string from the socket (state-dependent).
    virtual UINT read(std::string& str, const std::string& delimiter="\r\n");
    /** Send an IPC message over the socket (state-dependent)
     *  The IpcStruct data packet will be sent across the socket
     *  connection, either synchronously or asynchronously depending
     *  on the I/O mode (see the SetAsyncMode function).  If the
     *  mode is synchronous, then the packet will be first sent
     *  over the socket before this routine returns.  In the
     *  asynchronous mode, a worker thread will be made to do the
     *  write and this routine returns immediately.  When the worker
     *  thread completes, the application's callback routine is called.
     *  If the pCallback parameter is 0, then the routine that
     *  was given as parameter to registerCallback() will be called.
     *  Otherwise, if pCallback is specified, then it will be
     *  called instead.
     *
     *  Note that the IpcStruct data packet will be sent to the peer of
     *  the socket connection.  The static packet sequence number is
     *  increased by 1.
     *
     *  @param pData      Pointer to IPC structure (or sub-class)
     *  @param pCallback  Optional pointer to application's one-time-only callback.
     *  @return 0 on success
     */
    virtual int remoteProcedure(IpcStruct* pData, Callback pCallback = 0);
    //! Read an IPC message or reply from socket (state-dependent)
    virtual int remoteReadData(IpcStruct* pData, UINT uMaxLength = 0);
    //!     RemoteWriteReply Send a reply to an IPC message (state-dependent)
    virtual int remoteWriteReply(IpcReplyStruct* pData, DWORD dwSequence = 0);
    //!   Asynchronous I/O mode on or off.
    virtual void setAsyncMode(const bool bMode);
    //!   Set socket options.
    int setSockOpt(int nOptionName, const void* pOptionValue,
                   int nOptionLen, int nLevel = SOL_SOCKET);
    //! Write to socket (state-dependent)
    virtual void write(const void* pBuf, UINT uCount);
    //!  Write a string to the stream (state-dependent).
    virtual void write(const std::string& str);

    /** Returns a static, textual representation of an address
     *  (i.e., "host.acme.com:1074").  The value returned is an internal
     *  static and is thus not thread safe.
     */
    virtual operator const char* () const;
    //!   Assignment operator
    Socket& operator=(const Socket& rSource);

protected:
    Stream* listenIntern(Socket* pClient, const int nBacklog);

public:
    /** Open flags. */
    static constexpr int modeCreate       = 1;  //!< Create a server socket
    static constexpr int modeAsyncSocket  = 2;  //!< Socket will be asynchronous by default
    static constexpr int modeRead         = 4;  //!< Socket can only be read from
    static constexpr int modeWrite        = 8;  //!< Socket can only be written to
    static constexpr int modeReadWrite    = 16; //!< Socket can be read from and written to

protected:
    SocketAddr::AddrType m_PeerAddr;
    UINT m_uOpenFlags;
    bool m_bAsyncMode;
    UINT m_nProtocol;
    unsigned short int m_nFamily;
#if USE_OPENSSL
    SSL*     m_pSsl;
    SSL_CTX* m_pSslCtx;
#endif

private:
    // Counter for IPC messages (generates magic cookies)
    static DWORD m_dwSequence;

private:
    /// Do initializations that are common to all constructors.
    void initialize();

    // Disable copy constructor
    Socket(const Socket&) = delete;

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

    // Goes to the next specified state.
    void changeState(SocketState* pState);
    SocketState* m_pState;		// Pointer to current state
};

}  // namespace sockstr
