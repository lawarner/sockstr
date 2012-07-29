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

#ifndef _STREAMBUF_H_INCLUDED_
#define _STREAMBUF_H_INCLUDED_

#include <streambuf>


namespace sockstr
{
/*!
  @class StreamBuf
  Class that provides std::streambuf implementation for sockets.
*/

//
// FORWARD CLASS DECLARATIONS
//
class Stream;
#ifndef EOF
# define EOF (-1)
#endif


class StreamBuf : public std::streambuf
{
public:
    StreamBuf();
    StreamBuf(Stream* strm);
    virtual ~StreamBuf();

    StreamBuf* open(Stream* strm);

protected:
    virtual int overflow(int ch = EOF);
    virtual int pbackfail(int ch = EOF);
    virtual int uflow();
    virtual int underflow();
    virtual int sync();

private:
    Stream* stream;

    char inbuff[256];
    char outbuff[256];
};

}
#endif
