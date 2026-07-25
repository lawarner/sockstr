/*
   Copyright (C) 2026
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

// sstypes-macos.h: typedefs and defines used by SockStr classes
// Need to copy this file to sstypes.h for MacOS platform.
//

#ifndef _SSTYPES_H_INCLUDED_
#define _SSTYPES_H_INCLUDED_

// MacOS is similar to Linux
#ifndef TARGET_LINUX
#define TARGET_LINUX
#endif

#define USE_OPENSSL 1
#define LOOKUP_ACTIVE_INTERFACE 1

typedef int WORD;
typedef int DWORD;
typedef void * LPVOID;
typedef unsigned int UINT;

#define LPCSTR char *
#define SOCKET int
static const int INVALID_SOCKET = -1;
static const int SOCKET_ERROR = -1;

#define IOCTLSOCK ::ioctl

// Cross-platform compatibility wrappers
#ifndef IPV6_ADD_MEMBERSHIP
#define IPV6_ADD_MEMBERSHIP IPV6_JOIN_GROUP
#endif

#ifndef IPV6_DROP_MEMBERSHIP
#define IPV6_DROP_MEMBERSHIP IPV6_LEAVE_GROUP
#endif

#endif
