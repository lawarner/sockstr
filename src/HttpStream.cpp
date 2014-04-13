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

#include <sockstr/HttpHelpers.h>
#include <sockstr/HttpStream.h>
#include <cstdlib>
#include <string.h>
#include <sstream>
using namespace sockstr;

#define HTTP_VERSION_LINE " " HTTP_VERSION "\r\n"

const char* HttpStream::defaultHeaderFields_[] =
{
    "Accept", "*/*",
    0, 0
};

const char* HttpServerStream::defaultSrvHeaderFields_[] =
{
    "Date", "*",
    "Host", "*",
    0, 0
};


HttpStream::HttpStream()
    : Socket()
{
//    loadDefaultHeaders();
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
    std::string httpreq = "GET " + uri + HTTP_VERSION_LINE;
    expandHeaders(httpreq);
    write(httpreq);
    UINT ret = read(buffer, uCount);	//TODO loop for 1024 and fill string param

    return ret;
}

UINT HttpStream::head(const std::string& uri)
{
    return 0;
}

UINT HttpStream::post(const std::string& uri, char* message, char* buffer, UINT uCount)
{
    std::string httpreq = "POST " + uri + HTTP_VERSION_LINE;
    expandHeaders(httpreq);
    write(httpreq);
    if (message)
        write(message, strlen(message));

    UINT ret = read(buffer, uCount);	//TODO loop for 1024 and fill string param

    return ret;
}

UINT HttpStream::put(const std::string& uri, char* message, char* buffer, UINT uCount)
{
    std::string httpreq = "PUT " + uri + HTTP_VERSION_LINE;
    expandHeaders(httpreq);
    write(httpreq);
    if (message)
        write(message, strlen(message));

    UINT ret = read(buffer, uCount);	//TODO loop for 1024 and fill string param

    return ret;
}

UINT HttpStream::deleter(const std::string& uri)
{
    std::string httpreq = "DELETE " + uri + HTTP_VERSION_LINE;
    expandHeaders(httpreq);
    write(httpreq);
    char buffer[1024];
    UINT ret = read(buffer, sizeof(buffer));

    return ret;
}

void HttpStream::addHeader(const std::string& header, int value)
{
    std::ostringstream oss;
    oss << value;
    addHeader(header, oss.str());
}

void HttpStream::addHeader(const std::string& header, const std::string& value)
{
    std::string hdrstr = header;
    HttpParamEncoder* encoder = new FixedStringEncoder(value);
    headers_[hdrstr] = encoder;
}

void HttpStream::addHeader(const std::string& header, HttpParamEncoder* encoder,
                           const std::string& value)
{
    std::string hdrstr = header;
    headers_[hdrstr] = encoder;
}


void HttpStream::clearHeaders(void)
{
    headers_.clear();
}

void HttpStream::expandHeaders(std::string& str)
{
    HeaderMap::iterator it;
    for (it = headers_.begin(); it != headers_.end(); ++it)
    {
        HttpParamEncoder* encoder = it->second;
        std::string param = encoder->toString();
        str += it->first + ": " + param + "\r\n";
    }
    str += "\r\n";
}

void HttpStream::loadDefaultHeaders(void)
{
    for (int i = 0; defaultHeaderFields_[i]; i += 2)
    {
        if (strcmp("Host", defaultHeaderFields_[i]) == 0) {
            addHeader("Host", new HostnameEncoder(*this));
        } else if (strcmp("Date", defaultHeaderFields_[i]) == 0) {
            addHeader("Date", new TimestampEncoder(true));
        } else {
            addHeader(defaultHeaderFields_[i], defaultHeaderFields_[i+1]);
        }
    }
}


HttpServerStream::HttpServerStream()
    : HttpStream()
    , status_(*new HttpStatus)
{
}

HttpServerStream::HttpServerStream(const char* lpszFileName, UINT uOpenFlags)
    : HttpStream(lpszFileName, uOpenFlags)
    , status_(*new HttpStatus)
{
}

HttpServerStream::HttpServerStream(SocketAddr& rSockAddr, UINT uOpenFlags)
    : HttpStream(rSockAddr, uOpenFlags)
    , status_(*new HttpStatus)
{
}

HttpServerStream::~HttpServerStream()
{
    delete &status_;
}

Stream *
HttpServerStream::listen(const int nBacklog)
{
	// Construct a new client socket object
	Socket * pClient = new HttpServerStream;
    return listenIntern(pClient, nBacklog);
}

void HttpServerStream::loadDefaultHeaders(void)
{
    for (int i = 0; defaultSrvHeaderFields_[i]; i += 2)
    {
        if (strcmp("Host", defaultSrvHeaderFields_[i]) == 0) {
            addHeader("Host", new HostnameEncoder(*this));
        } else if (strcmp("Date", defaultSrvHeaderFields_[i]) == 0) {
            addHeader("Date", new TimestampEncoder(true));
        } else {
            addHeader(defaultSrvHeaderFields_[i], defaultSrvHeaderFields_[i+1]);
        }
    }
}

UINT HttpServerStream::response(const char* buffer, UINT uCount, const char* contentType,
    UINT statusCode)
{
    // send status line, headers, <blanks>, payload
    status_.setStatus(statusCode);

    if (contentType) addHeader("Content-Type", contentType);
    addHeader("Content-Length", uCount);

    std::string httpres = status_.statusLine();

    expandHeaders(httpres);
    write(httpres);
    if (buffer)
        write(buffer, uCount);

    return 0;
}
