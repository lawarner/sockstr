/*
   Copyright (C) 2012, 2013
   Andy Warner
   This file is part of the sockstr class library.

   The sockstr class library and other related programs such as ipctest is free 
   software; you can redistribute it and/or
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

#ifndef _HTTPSTREAM_H_INCLUDED_
#define _HTTPSTREAM_H_INCLUDED_
//
//

//
// INCLUDE FILES
//
#include <sockstr/Socket.h>


namespace sockstr
{

//
// MACRO DEFINITIONS
//
#ifndef DllExport
#define DllExport
#endif

//
// FORWARD CLASS DECLARATIONS
//

//
// TYPE DEFINITIONS
//

//
// CLASS DEFINITIONS
//
class DllExport HttpStream : public Socket
{
public:
    HttpStream();
	HttpStream(const char* lpszFileName, UINT uOpenFlags);
	HttpStream(SocketAddr& rSockAddr, UINT uOpenFlags);

    ~HttpStream();

    UINT get(const std::string& uri, const char* buffer);

private:
    HttpStream(const HttpStream&);	// disable copy constructor
    HttpStream& operator=(const HttpStream& rSource);	// disable assignment operator
};

}  // namespace sockstr

#endif // _HTTPSTREAM_H_INCLUDED_
