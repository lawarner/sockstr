/*
   Copyright (C) 2012 - 2021
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

#include <iostream>
#include <string>
#include <sockstr/sstypes.h>
#include <sockstr/StreamBuf.h>

/*
 * Socket class library.
 */
namespace sockstr {
//
// MACRO DEFINITIONS
//
#ifndef DllExport
#define DllExport
#endif

//
// TYPE DEFINITIONS
//
/**
 *  Stream status codes.
 *  These are used by the @deprecated queryStatus method.
 */
enum STATUSCODE {
    SC_OK,		//!< Stream status is good.
    SC_NODATA,	//!< No data is available on stream (useful for asynchronous streams.
    SC_FAILED	//!< Stream has an error.
};


/**
 *  @typedef Callback
 *  Definition of the asynchronous callback routines
 *  @param id  Corresponding request ID
 *  @param ptr Pointer to user data
 */
typedef void (*Callback)(DWORD id, void* ptr);

//
// FORWARD CLASS DECLARATIONS
//
class State;
class IpcStruct;
class IpcReplyStruct;

/**
 *  Stream base class.
 *
 *  The Stream class provides an abstract interface for the various
 *  transport-specific classes (see Socket for example).  This class
 *  is not specific to any particular transport mechanism.
 */
class DllExport Stream : public std::iostream {
public:
    virtual ~Stream() = default;     //!< Destructor

    /**  Indicate if the stream is open. */
    bool is_open() const;

    /**  Returns the current status of the stream.
     *   @deprecated Use good(), bad() and such functions from the 
     *       std::ios base class.
     */
    STATUSCODE queryStatus() const;

    /**  Register a user-defined callback function.
     *
     *   Register a user-defined function as the default routine for 
     *   this stream that will be called upon completion of asynchronous I/O.
     *   The previous callback is returned in case the caller wants to save/restore
     *   it.
     *   @param pCallback Address of the callback function
     *   @return the address of the previous callback is returned, or nullptr 
     *           if none was registered.
     */
    Callback registerCallback(Callback pCallback = nullptr);

    // Interfaces that are implemented by derived classes

    //!  Cancel any pending I/O operations on stream.
    virtual void abort() = 0;
    //!  Close the stream (state-dependent).
    virtual void close() = 0;
    //!  Listens and accepts incoming connections on a server-side stream.
    virtual Stream* listen(const int nBacklog = 4) = 0;
    //!  Open a stream (state-dependent).
    virtual bool open(const char* lpszFileName, UINT uOpenFlags) = 0;
    //!  Read raw data from the stream (state-dependent).
    virtual UINT read(void* pBuf, UINT uCount) = 0;
    //!  Read a string from the stream (state-dependent).
    virtual UINT read(std::string& str, int delimiter='\n') = 0;
    //!  Read a string from the stream (state-dependent).
    virtual UINT read(std::string& str, const std::string& delimiter="\r\n") = 0;
    /**  Indicates if stream can be reopened after a close (state-dependent).
     *
     *  This function may optionally be implemented by sub-classes that are
     *  able to reuse its instances for subsequent connections.  The Stream
     *  class implements this function by always returning false.
     */
    virtual bool reconnect();
    //!  Perform a remote procedure call over the stream (state-dependent).
    virtual int remoteProcedure(IpcStruct* pData, Callback pCallback = nullptr) = 0;
    /**  Receive either a remote procedure or a remote reply from the
     *   stream (state-dependent).
     */
    virtual int remoteReadData(IpcStruct* pData, 
                               UINT uMaxLength = 0) = 0;
    //!  Send a reply to a remote procedure call (state-dependent).
    virtual int remoteWriteReply(IpcReplyStruct* pData, 
                                 DWORD dwSequence) = 0;
    //!  Set the mode for subsequent I/O calls to either synchronous or asynchronous.
    virtual void setAsyncMode(const bool bMode) = 0;
    //!  Write raw data to the stream (state-dependent).
    virtual void write(const void* pBuf, UINT uCount) = 0;
    //!  Write a string to the stream (state-dependent).
    virtual void write(const std::string& str) = 0;
    //!  Return a static, textual representation of the peer's address.
    virtual operator const char* (void) const = 0;

protected:
    //! Constructs a Stream objects.
    //! This abstract class can only be instanciated from a sub-class.
    Stream();
    //! Constructs a Stream objects.
    //! This abstract class can only be instanciated from a sub-class.
    //! @param hFile an already opened stream handle that will be used
    //!              by this Stream.
    Stream(const UINT hFile);

    // Disable copy constructor and assignment operator
    Stream(const Stream&) = delete;
    Stream& operator=(const Stream&) = delete;

protected:
    SOCKET m_hFile;		//!< Handle to stream

    //! Pointer to the current state of the stream
    State* m_pState;
    //! Current status of the stream
    STATUSCODE m_Status;
    //! Address of the user's default callback routine for this stream
    Callback m_pDefCallback;

    //! Implements std::streambuf interface for this class.
    StreamBuf strbuf;
};

}  // namespace sockstr
