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

#ifndef _STREAM_H_INCLUDED_
#define _STREAM_H_INCLUDED_

#include <iostream>
#include <string>
#include <sockstr/StreamBuf.h>

namespace sockstr
{
/*!
  @class Stream
  The Stream class provides a transport-independent
      interface for IPC.  It is the abstract base class for
	  transport-specific classes such as Socket.
*/


//
// MACRO DEFINITIONS
//
#ifndef DllExport
#define DllExport
#endif

//
// TYPE DEFINITIONS
//
enum STATUSCODE
{
    SC_OK,
    SC_NODATA,
    SC_EOF,
    SC_FAILED
};


/*!
  @typedef Callback
  Definition of the asynchronous callback routines
  @param id  Corresponding request ID
  @param ptr Pointer to user data
*/
typedef void (*Callback)(DWORD id, void* ptr);

//
// FORWARD CLASS DECLARATIONS
//
class State;
class IpcStruct;
class IpcReplyStruct;

/*!
  Stream base class.
  The Stream class provides a transport-independent
      interface for IPC.  It is the abstract base class for
	  transport-specific classes such as Socket.
*/
class DllExport Stream : public std::iostream
{
public:
	virtual ~Stream(void);

	/*!  Returns the current status of the stream.
     *   @deprecated Use good(), bad() and such functions from the 
     *       std::ios base class.
     */
    STATUSCODE  queryStatus     (void) const;
	/*!
	     Register a user-defined callback function.
	     Register a user-defined function as the default routine for 
         this stream that will be called upon completion of asynchronous I/O.
         @param pCallback Address of the callback function
	*/
	Callback    registerCallback(Callback pCallback = 0);

    // Interfaces that are implemented by derived classes

	//!  Cancel any pending I/O operations on stream.
	virtual void    abort		   (void) = 0;
	//!  Close the stream (state-dependent).
    virtual void    close           (void) = 0;
	//!  Listens and accepts incoming connections on a server-side stream.
	virtual Stream*
				    listen          (const int nBacklog = 4) = 0;
	//!  Open a stream (state-dependent).
    virtual bool    open            (const char* lpszFileName, UINT uOpenFlags) = 0;
	//!  Read raw data from the stream (state-dependent).
    virtual UINT    read            (void* pBuf, UINT uCount) = 0;
	//!  Read a string from the stream (state-dependent).
    virtual UINT    read            (std::string& str, int delimiter='\n') = 0;
	//!  Indicates if stream can be reopened after a close (state-dependent).
    virtual bool    reconnect       (void);
	//!  Perform a remote procedure call over the stream (state-dependent).
	virtual int     remoteProcedure (IpcStruct* pData,
								 	 Callback pCallback = 0) = 0;
	//!  Receive either a remote procedure or a remote reply from the
	//!    stream (state-dependent).
	virtual int     remoteReadData  (IpcStruct* pData, 
				 					UINT uMaxLength = 0) = 0;
	//!  Send a reply to a remote procedure call (state-dependent).
	virtual int     remoteWriteReply(IpcReplyStruct* pData, 
									DWORD dwSequence) = 0;
	//!  Set the mode for subsequent I/O calls to either synchronous or asynchronous.
	virtual void    setAsyncMode    (const bool bMode) = 0;
	//!  Write raw data to the stream (state-dependent).
    virtual void    write           (const void* pBuf, UINT uCount) = 0;
	//!  Write a string to the stream (state-dependent).
    virtual void    write           (const std::string& str) = 0;
	//!  Return a static, textual representation of the peer's address.
	virtual operator char* (void) const = 0;

protected:
    Stream(void);
    Stream(const UINT hFile);

private:
	// Disable copy constructor and assignment operator
	Stream(const Stream&);
	Stream& operator=(const Stream&);

protected:
    SOCKET m_hFile;

	//! Pointer to the current state of the stream
    State*     m_pState;
	//! Current status of the stream
    STATUSCODE m_Status;
	//! Address of the user's default callback routine for this stream
	Callback    m_pDefCallback;

    StreamBuf strbuf;
};

}
#endif
