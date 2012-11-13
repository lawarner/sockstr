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

#ifndef _IPC_H_INCLUDED_
#define _IPC_H_INCLUDED_
//
// File       : IPC.h
//
// Class      : IpcStruct
//
// Hierarchy  : IpcStruct (base class)
//               |
//               +-- IpcReplyStruct
//
// Description: This header file is to be included by applications
//              that use the IPC class library.  The generic IPC
//              message layout, as well as the definition of all IPC
//              messages and replies used in the  application are
//              contained in this header file.
//
//              Note that this header file deviates (slightly) from the
//              coding standard by defining more than one class.  This
//              was deemed desirable since the 2 classes, IpcStruct
//              and IpcReplyStruct are so interrelated.  Another
//              advantage of including these classes together is that
//              all IPC message definitions can be grouped in one place,
//              namely, this header file.
//
// Members    :
//   Data
//
//     m_wFunction   Ordinal number of the IPC function call
//     m_wPacketSize Size in bytes of the whole IPC message
//     m_dwSequence  Magic cookie which uniquely identifies each remote
//                   package that is sent/received
//     m_lReturn;	 Return value from remote procedure
//
//   ----------------------------------------------------------------
//

//
// INCLUDE FILES
//
#include <sockstr/sstypes.h>


namespace sockstr
{

//
// MACRO DEFINITIONS
//
#ifndef DllExport
#define DllExport
#endif

#define SS_VERSION 0100	// Current version of protocol

#define IPC_SEPCHAR "\n"    // separator character for variable length fields
#define IPC_SEPTOKEN '\n'

#define IPC_MESSAGE(MSGNAME)				\
class MSGNAME : public IpcStruct			\
{											\
public:										\
    MSGNAME(void) : IpcStruct(IPC_ ## MSGNAME, sizeof(MSGNAME)) { }

#define IPC_ENDMESSAGE };

#define IPC_REPLY(MSGNAME)					\
class MSGNAME : public IpcReplyStruct	\
{											\
public:										\
    MSGNAME(void) : IpcReplyStruct(IPC_ ## MSGNAME + 10000, sizeof(MSGNAME)) { }

#define IPC_ENDREPLY };


// Note that UDP (broadcast) messages must all be fixed length
#define SS_UDP_SIZE 128

// Field length defines here.  Note that they should be used in the client code.
//  Do NOT use them in the IPC message definitions
//  because this will cause the parser in "ipcwiz" to fail!
#define LENGTH_CONNECTION_ID  30
#define LENGTH_DESTINATION_ID 30
#define LENGTH_PHONE_NUMBER   20

//
// TYPE DEFINITIONS
//
typedef DWORD CallHandle;

//
// FORWARD CLASS DECLARATIONS
//

//
// DATA DECLARATIONS
//

// Remarks  :  All IPC messages for the  system need to be defined here.
//             First, create a unique function ordinal.  The name for this
//             ordinal MUST have "IPC_" as prefix, followed by the name of
//             the message.  The actual message definition must be placed
//             in the CLASS DECLARATION section below.
//

// General messages:  1-99
static const WORD IPC_EndOfData                = 1;
static const WORD IPC_EndOfFile                = 2;
static const WORD IPC_GenericReply             = 3;
static const WORD IPC_SocketResponse           = 4;

// Application defined: 1001-4999
// static const WORD IPC_ConnectTo             = 1001;
//  ...

//
// FUNCTION DECLARATIONS
//

//
// CLASS DECLARATION
#pragma pack(2)

class DllExport IpcStruct
{
public:
	IpcStruct(void)
        : wFunction_(0)
        , wPacketSize_(0)
        , dwSequence_(0)
    { }
	IpcStruct(WORD wFunction, UINT wPacketSize, DWORD dwSequence = 0)
        : wFunction_(wFunction)
        , wPacketSize_(wPacketSize)
        , dwSequence_(dwSequence)
    { }

	WORD  wFunction_;
	UINT  wPacketSize_;
	DWORD dwSequence_;	// cookie
};

class DllExport IpcReplyStruct : public IpcStruct
{
public:
	IpcReplyStruct(WORD wFunction, UINT wPacketSize)
        : IpcStruct(wFunction, wPacketSize)
        , dwReturn_(0)
    { }

	DWORD dwReturn_;	// Return value from remote procedure
};


// General messages
IPC_MESSAGE(EndOfData)
    WORD wEndFunction_;
IPC_ENDMESSAGE

IPC_MESSAGE(EndOfFile)
    // Empty
IPC_ENDMESSAGE

IPC_MESSAGE(GenericReply)
	char filler_[2048];	    // This should be large enough to contain
							// the biggest message.
IPC_ENDMESSAGE

// Useful for handshake negotitation of port setup between server-side and client
IPC_MESSAGE(SocketResponse)
    WORD wResponse_;			// 0 = failure; 1 = success
    WORD wPort_;
    char szServer_[100];
IPC_ENDMESSAGE

// Application message here:
//IPC_MESSAGE(ConnectTo)
//    DWORD m_dwIdentity;
//    char  m_szName[24];
//IPC_ENDMESSAGE


#pragma pack()

}  // namespace sockstr

#endif
