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
{
    setg(inbuff, inbuff+sizeof(inbuff), inbuff+sizeof(inbuff));
    setp(outbuff, outbuff+sizeof(outbuff));
}

StreamBuf::StreamBuf(Stream* strm)
    : stream(strm)
{
    setg(inbuff, inbuff+sizeof(inbuff), inbuff+sizeof(inbuff));
    setp(outbuff, outbuff+sizeof(outbuff));
}

StreamBuf::~StreamBuf()
{

}


StreamBuf* StreamBuf::open(Stream* strm)
{
    stream = strm;
    return this;
}

// virtual protected members overridden from Standard C++ Library std::streambuf

int StreamBuf::overflow(int ch)
{
    char chr = (char) ch;
    if (stream)
    {
        if (sizeof(outbuff) < 2)
        {
            if (ch != EOF)
            {	// unbuffered
                stream->write(&chr, 1);
            }
            return ch;
        }

        if (ch != EOF && pptr() < epptr())
        {
            *pptr() = chr;
            pbump(1);
        }

        if (pbase() < pptr())
        {
            stream->write(pbase(), pptr() - pbase());
            if (stream->queryStatus() != SC_OK)
                return EOF;

            setp(outbuff, outbuff+sizeof(outbuff));
        }
    }
    else if (sizeof(outbuff) > 1)
    {
        setp(outbuff, outbuff+sizeof(outbuff));

        *this->pptr() = chr;
        this->pbump(1);
    }

    return ch;
}


int StreamBuf::pbackfail(int ch)
{
    char chr = (char) ch;
    if (eback() == gptr())
        return EOF;

    gbump(-1);
    if (ch != EOF)
    {
        *gptr() = chr;
    }

    return ch;
}

int StreamBuf::sync()
{
    if (pptr() == pbase())
        return EOF;

    if (stream)
    {
        stream->write(pbase(), pptr() - pbase());
        if (stream->queryStatus() != SC_OK)
            return EOF;

        setp(outbuff, outbuff+sizeof(outbuff));
        return 0;
    }

    return EOF;
}


int StreamBuf::uflow()
{
    char chr = underflow();
    if (gptr() < egptr())
        gbump(1);

    return chr;
}

int StreamBuf::underflow()
{
    char chr = EOF;
    if (stream)
    {
        int sz = stream->read(inbuff, sizeof(inbuff));
        if (sz < 1)
            return EOF;
        setg(inbuff, inbuff, inbuff + sz);

        chr = *gptr();
    }
    return chr;
}

