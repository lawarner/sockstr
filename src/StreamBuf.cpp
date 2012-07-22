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
// File       : StreamBuf.cpp
//
// Class      : StreamBuf
//
// Description: Subclass of std::streambuf that uses sockets.
//

//
// INCLUDE FILES
//
#include "config.h"
#include <cassert>

#include <sockstr/StreamBuf.h>
#include <sockstr/Stream.h>

using namespace sockstr;

//
// MACRO DEFINITIONS
//

//
// TYPE DEFINITIONS
//


//
// CLASS MEMBER FUNCTION DEFINITIONS
//

StreamBuf::StreamBuf()
    : stream(0)
    , unputbuf(EOF)
{

}

StreamBuf::StreamBuf(Stream* strm)
    : stream(strm)
    , unputbuf(EOF)
{

}


StreamBuf* StreamBuf::open(Stream* strm)
{
    stream = strm;
    return this;
}

// virtual protected memers overridden from Standard C++ Library std::streambuf

int StreamBuf::overflow(int ch)
{
    char chr = (char) ch;
    if ((ch!=EOF) && stream)
        stream->write(&chr, 1);

    return ch;
}

int StreamBuf::pbackfail(int ch)
{
    if (unputbuf == EOF)
        unputbuf = ch;
    else
        return EOF;

    return ch;
}

int StreamBuf::uflow()
{
    return readChar();
}

int StreamBuf::underflow()
{
    char chr = EOF;
    if (stream)
    {
        chr = readChar();
        pbackfail(chr);
    }
    return chr;
}


int StreamBuf::readChar()
{
    char chr = EOF;
    if (stream)
    {
        if (unputbuf != EOF)
        {
            chr = unputbuf;
            unputbuf = EOF;
        }
        else
            stream->read(&chr, 1);
    }
    return chr;
}
