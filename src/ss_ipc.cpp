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

// ss_ipc.cpp : Defines the initialization routines for the DLL.
//

#include "config.h"
#include <sockstr/IPC.h>
#include <sockstr/SocketState.h>

using namespace sockstr;

// Abstract : Constructs a IpcStruct object
//
// Returns  : -
// Params   :
//   wFunction                 Ordinal for the function call
//   wPacketSize               Size in bytes of the entire structure
//
// Pre      :
// Post     : A IpcStruct object is made and the function call ordinal and
//            packet size member variables are filled in from the constructor's
//            parameters.  The sequence member variable is initially set to 0.
//
// Remarks  :
//
IpcStruct::IpcStruct(const WORD wFunction, const UINT wPacketSize)
: m_wFunction  (wFunction)
, m_wPacketSize(wPacketSize)
, m_dwSequence (0)
{

}


// Abstract : Constructs a IpcReplyStruct object
//
// Returns  : -
// Params   :
//   wFunction                 Ordinal for the function call
//   wPacketSize               Size in bytes of the entire structure
//
// Pre      :
// Post     : A IpcReplyStruct object is made and the function call ordinal
//            and packet size member variables are filled in from the
//            constructor's parameters.  The sequence and the return member
//            variables are initially set to 0.
//
// Remarks  :
//
IpcReplyStruct::IpcReplyStruct(const WORD wFunction, const UINT wPacketSize)
: IpcStruct(wFunction, wPacketSize)
, m_lReturn   (0)
{

}
