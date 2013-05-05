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

// HttpStream.cpp
//

#include <sockstr/HttpStream.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
using namespace sockstr;


static const char* defaultHeaderFields[] =
{
    "Accept", "*/*",
    0, 0
};



CompoundEncoder::CompoundEncoder()
{

}

std::string CompoundEncoder::toString()
{
    std::string str;
    std::vector<HttpParamEncoder*>::iterator it;
    for (it = encoders_.begin(); it != encoders_.end(); ++it)
    {
        str += (*it)->getName() + "=\"" + (*it)->toString() + "\", ";
    }
    return str;
}

void CompoundEncoder::addElement(HttpParamEncoder* encoder)
{
    encoders_.push_back(encoder);
}


TimestampEncoder::TimestampEncoder(time_t timeSecs)
    : timeSecs_(timeSecs)
{

}

std::string TimestampEncoder::toString()
{
    char outstr[200];
    struct tm* tmp;
    tmp = localtime(&timeSecs_);

    strftime(outstr, sizeof(outstr), "%a, %d %b %Y %T %Z", tmp);
    return std::string(outstr);
}


HttpStream::HttpStream()
    : Socket()
{

}

HttpStream::HttpStream(const char* lpszFileName, UINT uOpenFlags)
    : Socket(lpszFileName, uOpenFlags)
{
    loadDefaultHeaders();
}

HttpStream::HttpStream(SocketAddr& rSockAddr, UINT uOpenFlags)
    : Socket(rSockAddr, uOpenFlags)
{
    loadDefaultHeaders();
}

HttpStream::~HttpStream()
{

}

UINT HttpStream::get(const std::string& uri, char* buffer, UINT uCount)
{
    std::string httpreq = "GET " + uri + " HTTP/1.1\r\n";
    expandHeaders(httpreq);
    write(httpreq);
    UINT ret = read(buffer, uCount);	//TODO loop for 1024 and fill string param
    std::cout << "header:" << std::endl << httpreq << std::endl
              << "read=" << ret << std::endl;

    return ret;
}

UINT HttpStream::head(const std::string& uri)
{
    return 0;
}

UINT HttpStream::post(const std::string& uri, char* message, char* buffer, UINT uCount)
{
    std::string httpreq = "POST " + uri + " HTTP/1.1\r\n";
    expandHeaders(httpreq);
    write(httpreq);
    if (message)
        write(message, strlen(message));

    UINT ret = read(buffer, uCount);	//TODO loop for 1024 and fill string param
    std::cout << "header:" << std::endl << httpreq << std::endl
              << "read=" << ret << std::endl;

    return ret;
}

UINT HttpStream::put(const std::string& uri, char* buffer)
{
    return 0;
}

UINT HttpStream::deleter(const std::string& uri)
{
    std::string httpreq = "DELETE " + uri + " HTTP/1.1\r\n";
    expandHeaders(httpreq);
    write(httpreq);
    char buffer[1024];
    UINT ret = read(buffer, sizeof(buffer));
    std::cout << "header:" << std::endl << httpreq << std::endl
              << "read=" << ret << std::endl;

    return ret;
}

void HttpStream::addHeader(const std::string& header, const std::string& value)
{
    std::string hdrstr = header;
    HttpHeader* hdr = new HttpHeader;
    hdr->value   = value;
    hdr->encoder = 0;
    headers[hdrstr] = hdr;
}

void HttpStream::addHeader(const std::string& header, HttpParamEncoder* encoder,
                           const std::string& value)
{
    std::string hdrstr = header;
    HttpHeader* hdr = new HttpHeader;
    hdr->value   = value;
    hdr->encoder = encoder;
    headers[hdrstr] = hdr;
}


void HttpStream::clearHeaders(void)
{
    headers.clear();
}

void HttpStream::expandHeaders(std::string& str)
{
    HeaderMap::iterator it;
    for (it = headers.begin(); it != headers.end(); ++it)
    {
        HttpHeader* hdr = it->second;
        std::string param;
        if (hdr->encoder)
            param = hdr->encoder->toString();
        else
            param = hdr->value;

        str += it->first + ": " + param + "\r\n";
    }
    str += "\r\n";
}

void HttpStream::loadDefaultHeaders(void)
{
    for (int i = 0; defaultHeaderFields[i]; i += 2)
    {
        addHeader(defaultHeaderFields[i], defaultHeaderFields[i+1]);
    }
#if 0
    std::string strhost((const char *) *this);
    strhost = strhost.substr(0, strhost.find_first_of(':'));
    if (!strhost.empty())
        addHeader("Host", strhost);
#endif
}
