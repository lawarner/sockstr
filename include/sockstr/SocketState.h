/*
   Copyright (C) 2012, 2013, 2023
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

namespace sockstr {
/**
 *  @class SocketState
 * 
 *  Class Hierarchy  : SocketState (base class)
 *                |
 *                +--SSClosed
 *                +--SSOpenedServer
 *                +--SSOpenedClient
 *                +--SSListening
 *                +--SSConnected
 * 
 *               Note that this header file deviates from the normal coding
 *               standards in that it includes more than one class definition.
 *               It was deemed desirable to have all of these state classes
 *               together.  Otherwise, the overview of the functionality that
 *               this state tree performs is obscured.
 * 
 */
//   ----------------------------------------------------------------
//                   (Note that most of the overridables in this class 
//                   are state-dependent functions)
//
//
// History    : A. Warner, 1996-05-01, Creation
//              A. Warner, 2012-07-07, Added doxygen comments
//

//
// FORWARD CLASS DECLARATIONS
//
struct IOPARAMS;
class Socket;
class SocketAddr;

//! Base class for a Socket's state tree.
class SocketState {
public:
    //! Constructs a SocketState object
    SocketState() = default;
    //! Destructs a SocketState object
    ~SocketState() = default;

    // Disable copy constructor and assignment operator
    SocketState(const SocketState&) = delete;
    SocketState& operator=(const SocketState&) = delete;

    // State-dependent functions
    //! Cancel any pending I/O operations on stream
    virtual void   abort       (Socket* pSocket);
    //! Close the socket connection, if open
    virtual void   close       (Socket* pSocket);
    /** Create a new server socket connection (ready
     *  to do a Listen()) from an already open connection.
     *
     *  NOT YET IMPLEMENTED */
    //virtual Socket& createServer(Socket* pSocket, const WORD  wPort, const IPAddress* pHost);

    //! Get the socket options currently in effect
    virtual bool   getSockOpt  (Socket* pSocket,
                                int  nOptionName, void* pOptionValue,
                                socklen_t* nOptionLen,  int   nLevel);
    //! Listen on a server socket for incoming connections
    virtual SOCKET listen      (Socket* pSocket, const int nBacklog);
    //! Open the socket connection
    virtual bool   open        (Socket* pSocket,
                                SocketAddr& rSockAddr,
                                UINT uOpenFlags);
    //! Read raw data from socket stream
    virtual UINT   read        (Socket* pSocket, void* pBuf, UINT uCount);
    //! Reader worker thread processing routine.
    virtual DWORD  readerThread(IOPARAMS* pIOP);
    //! Set a socket option to a new value
    virtual bool   setSockOpt  (Socket* pSocket,
                                int nOptionName, const void* pOptionValue,
                                int nOptionLen, int nLevel);
    //! Write raw data to socket stream
    virtual void   write       (Socket* pSocket, const void* pBuf, UINT uCount);
    //! Writer worker thread processing routine
    virtual DWORD  writerThread(IOPARAMS* pIOP);


    void read_thread_handler(IOPARAMS* pIOP);
    void write_thread_handler(IOPARAMS* pIOP);

protected:
    //! Goes to the next specified state
    void changeState(Socket* pSocket, SocketState* pState);
    /** Sets the socket object pointer, buffer, buffer size and
     *  callback for the worker thread.  This protected routine
     *  is only used internally by this class. */
    IOPARAMS*
    createIOParams(Socket* pSocket, void* pBuf, UINT uCount,
                   Callback pCallback);
    IOPARAMS*
    createIOParams(Socket* pSocket, const void* pBuf, UINT uCount,
                   Callback pCallback);
};


//////////////////////////////////////////////////////////
//   All subclasses of SocketState are defined here.

class SSClosed : public SocketState {
public:
    static SocketState* instance();

private:
    static SocketState* m_pInstance;
};


class SSOpenedServer : public SocketState {
public:
    static SocketState* instance();

    virtual bool open(Socket* pSocket,
                      SocketAddr& rSockAddr,
                      UINT uOpenFlags);

private:
    static SocketState* m_pInstance;
};


class SSOpenedClient : public SocketState {
public:
    static SocketState* instance();

    virtual bool open(Socket* pSocket,
                      SocketAddr& rSockAddr,
                      UINT uOpenFlags);

private:
    static SocketState* m_pInstance;
};


class SSListening : public SocketState {
public:
    static SocketState* instance();

    virtual SOCKET listen(Socket* pSocket, const int nBacklog);

private:
    static SocketState* m_pInstance;
};


class SSConnected : public SocketState {
public:
    static  SocketState* instance(void);

    virtual UINT read(Socket* pSocket, void* pBuf, UINT uCount);
    virtual DWORD readerThread(IOPARAMS* pIOP);
    virtual void write(Socket* pSocket, const void* pBuf, 
                       UINT uCount);
    virtual DWORD writerThread(IOPARAMS* pIOP);

private:
    int readSocket(Socket* pSocket, void* pBuf, UINT uCount);

#ifdef _DEBUG
    static void* m_pLastBuffer;	// Last buffer used for overlapped I/O
#endif
    static SocketState* m_pInstance;
};


#if USE_OPENSSL

class SSOpenedClientTLS : public SocketState {
private:
    SSOpenedClientTLS();

public:
    static SocketState* instance(void);

    //! Open the socket connection
    virtual bool   open        (Socket* pSocket,
                                SocketAddr& rSockAddr,
                                UINT uOpenFlags);

    //! Get the socket options currently in effect
    virtual bool   getSockOpt  (Socket* pSocket,
                                int  nOptionName, void* pOptionValue,
                                socklen_t* nOptionLen,  int   nLevel);
    //! Set a socket option to a new value
    virtual bool   setSockOpt  (Socket* pSocket,
                                int nOptionName, const void* pOptionValue,
                                int nOptionLen, int nLevel);

private:
    std::string m_key;
    std::string m_password;
    std::string m_cafile;
    std::string m_capath;

private:
	static SocketState* m_pInstance;
};


class SSConnectedTLS : public SocketState {
public:
    static  SocketState* instance();

    virtual void close(Socket* pSocket);
    virtual UINT read(Socket* pSocket, void* pBuf, UINT uCount);
    virtual DWORD readerThread(IOPARAMS* pIOP);
    virtual void write(Socket* pSocket, const void* pBuf, 
                       UINT uCount);
    virtual DWORD writerThread(IOPARAMS* pIOP);

private:
    int readSocket(Socket* pSocket, void* pBuf, UINT uCount);

    static SocketState* m_pInstance;
};

#endif // USE_OPENSSL

}  // namespace sockstr
