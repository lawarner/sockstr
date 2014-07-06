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
// File       : Stream.cpp
//
// Class      : Stream
//
// Description: The Stream class provides a transport-independent
//              interface for IPC.  It is the abstract base class for
//              transport-specific classes such as Socket.
//
// Decisions  : This class was originally based on the MFC CFile class and should
//              be used as the base class of all further transport classes
//              such as Socket.  The main reason why a transport
//              class should inherit from Stream instead of from CFile
//              is: Stream has extended the CFile class with such
//              functionality as non-blocking I/O, a remote procedure
//              call mechanism, and an extendable state machine.
//              And it is polymorphic, which CFile is lacking.
//
//   Note : The newer versions are based on the std::iostream class.

//
// INCLUDE FILES
//
#include "config.h"
#include <sockstr/Stream.h>

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
// FUNCTION DEFINITIONS
//

//
// CLASS MEMBER FUNCTION DEFINITIONS
//

// Function :
//   Stream(void)
//
//   Stream(const UINT uFile)
//
// Abstract : Constructs a Stream object
//
// Returns  : -
// Params   :
//   uFile                     Handle to an already open stream.
//
// Pre      : These constructors are called while creating an instance
//            of a sub-class.  The second form of constructor assumes
//            that the sub-class has already opened the stream.
// Post     : A Stream object is made.  The m_Status variable is 
//            initialized to SC_OK and m_pDefCallback is set to 0 (initially
//            there is no callback).
//
// Remarks  : Note that Stream is an abstract base class and cannot be
//            instantiated directly.  For this reason, these constructors
//            are protected (instead of public).
//
Stream::Stream(void)
    :	std::iostream (&strbuf)
    ,	m_hFile       (INVALID_SOCKET)
    ,   m_Status      (SC_OK)
    ,	m_pDefCallback(0)
    ,	strbuf(this)
{
    clear();
}


Stream::Stream(const UINT uFile)
    :	std::iostream (&strbuf)
    ,	m_hFile       (uFile)
	,	m_Status      (SC_OK)
    ,	m_pDefCallback(0)
    ,	strbuf(this)
{
    clear();
}


// 
// Function :
//   virtual ~Stream(void)
//
// Abstract : Destructor of Stream object
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
Stream::~Stream(void)
{

}


bool Stream::is_open(void) const
{
    return m_hFile != INVALID_SOCKET;
}

// Abstract : Returns the current status of the stream.
//
// Returns  : STATUSCODE (enum defined in Stream.hpp)
//
// Pre      :
// Post     :
//
// Remarks  : Note that the status of a stream should not be confused
//            with its state.  For example, a stream's state can be
//            "connected" with a status of "NoData".
//            This function is deprecated as the std::ios base class functions
//            and status bits should be used.
//
STATUSCODE
Stream::queryStatus(void) const
{
    return m_Status;
}


// Abstract : Indicates if stream can be reused for subsequent connections.
//
// Returns  : boolean
// Params   :
//   -
//
// Pre      :
// Post     :
//
// Remarks  :
//
bool
Stream::reconnect(void)
{
	return false;
}


// Abstract : Stores the address of the user's callback routine,
//            returning the previously registered callback.
//
// Returns  : The previously registered callback routine, or 0 if none.
// Params   :
//   pCallback                 Address of application's callback routine
//
// Pre      :
// Post     : The address of the user's callback will be stored in 
//            m_pDefCallback for later use.
//
// Remarks  :
//
Callback
Stream::registerCallback(Callback pCallback)
{
	Callback pOldCallback = m_pDefCallback;
	m_pDefCallback = pCallback;
	return pOldCallback;
}

//
// END OF FILE
//
