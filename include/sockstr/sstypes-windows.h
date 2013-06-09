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

// sstypes-windows.h: typedefs and defines used by SockStr classes
// Need to copy this file to sstypes.h for Windows platform.
//

#ifndef _SSTYPES_H_INCLUDED_
#define _SSTYPES_H_INCLUDED_

#ifndef TARGET_WINDOWS
#define TARGET_WINDOWS
#endif

#define NOMINMAX 1

#define _X86_
#include <WinDef.h>
#include <WinSock2.h>

#define IOCTLSOCK ::ioctlsocket

#ifndef strcasecmp
#define strcasecmp _stricmp
#endif

#endif
