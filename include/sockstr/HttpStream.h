/*
   Copyright (C) 2012, 2013, 2023
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
// INCLUDE FILES
//
#include <sockstr/Socket.h>
#include <map>
#include <vector>

#include <time.h>


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
class HttpParamEncoder;
class HttpStatus;

//
// TYPE DEFINITIONS
//

//
// CLASS DEFINITIONS
//

/**
 * Class to handle client-side HTTP protocol over a socket connection.
 */
class DllExport HttpStream : public Socket {
protected:
    typedef std::map<std::string, HttpParamEncoder*> HeaderMap;

public:
    HttpStream();
    HttpStream(const char* lpszFileName, UINT uOpenFlags);
    HttpStream(SocketAddr& rSockAddr, UINT uOpenFlags);

    virtual ~HttpStream();

    UINT get(const std::string& uri, std::string& content, std::string& headers);

    UINT get(const std::string& uri, char* buffer, UINT uCount);
    UINT head(const std::string& uri);
    UINT post(const std::string& uri, char* message, char* buffer, UINT uCount);
    UINT put(const std::string& uri, char* message, char* buffer, UINT uCount);
    UINT deleter(const std::string& uri);

    void addHeader(const std::string& header, int value);
    void addHeader(const std::string& header, const std::string& value);
    void addHeader(const std::string& header, HttpParamEncoder* encoder,
                   const std::string& value = "");
    void clearHeaders(void);
    void expandHeaders(std::string& str);
    virtual void loadDefaultHeaders(void);
    void parseHeaders(const char* buffer, UINT uSize, HeaderMap& headers);

protected:
    HeaderMap headers_;

protected:
    static const char* defaultHeaderFields_[];

private:
    HttpStream(const HttpStream&);	// disable copy constructor
    HttpStream& operator=(const HttpStream& rSource);	// disable assignment operator
};

/**
 * Class to handle server-side HTTP protocol over a socket connection.
 */
class DllExport HttpServerStream : public HttpStream {
public:
    HttpServerStream();
    HttpServerStream(const char* lpszFileName, UINT uOpenFlags);
    HttpServerStream(SocketAddr& rSockAddr, UINT uOpenFlags);

    virtual ~HttpServerStream();

	virtual Stream* listen(const int nBacklog = 4);
    virtual void loadDefaultHeaders(void);

    enum HttpFunction
    {
        INVALID = -1,
        DELETE,
        GET,
        HEAD,
        OPTIONS,
        PUT,
        POST
    };

    static const char* functionName(HttpFunction function);

    HeaderMap getRequestHeaders() const;

    UINT response(const char* buffer, UINT uCount, 
                  const char* contentType = 0, UINT statusCode = 200);
    UINT request(char* buffer, UINT uCount,
                 HttpFunction& funct, std::string& url);

protected:
    std::vector<std::string>& split(const std::string &s, char delim,
                                    std::vector<std::string> &elems);
    std::vector<std::string>  split(const std::string &s, char delim);


protected:
    HttpStatus& status_;
    /** Request headers */
    HeaderMap reqHeaders_;

protected:
    static const char* defaultSrvHeaderFields_[];
};

}  // namespace sockstr

#endif // _HTTPSTREAM_H_INCLUDED_
