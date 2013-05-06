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

//
// TYPE DEFINITIONS
//

//
// CLASS DEFINITIONS
//
/**
 * The HTTP header contains parameters that are the form "Name: Value"
 * The "Value" portion can be a fixed string, but often it is a computed
 * value, for example the Date parameter.  The HttpParamEncoder class can be
 * subclassed to handle these special computed, encoded parameter values.
 */
class DllExport HttpParamEncoder
{
public:
    HttpParamEncoder() { }
    HttpParamEncoder(const std::string& name, const std::string& value = std::string())
        : name_(name), value_(value) { }
    virtual ~HttpParamEncoder() { }

    virtual void set(const std::string& value) { value_ = value; }
    virtual std::string toString() { return value_; }
    virtual std::string getNameValue()
    {
        return getName() + "\"" + toString() + "\"";
    }

    void setName(const std::string& name) { name_ = name; }
    const std::string& getName() const { return name_; }

private:
    std::string name_;	// optional name
    std::string value_;	// optional value
};

/**
 * The CompoundEncoder is simply a list of other encoders.
 * This is useful when a parameter contains multiple parts.  For
 * example, an Authorization header containing values for OAuth.
 */
class DllExport CompoundEncoder : public HttpParamEncoder
{
public:
    CompoundEncoder(const char* separator = ", ");
    virtual ~CompoundEncoder();
    virtual std::string toString();

    /**
     * Add encoder to this compound encoder.
     * @param encoder Pointer to an http encoder.  Ownership of the memory 
     *                pointed to by encoder is transferred to this object
     *                and will be freed when this object is destroyed.
     */
    void addElement(HttpParamEncoder* encoder);

private:
    std::vector<HttpParamEncoder*> encoders_;
    const char* separator_;
};


class DllExport FixedStringEncoder : public HttpParamEncoder
{
public:
    /** Construct a FixedStringEncoder. */
    FixedStringEncoder(const std::string& value = std::string())
        : HttpParamEncoder(std::string(), value) { }
    FixedStringEncoder(const std::string& name, const std::string& value)
        : HttpParamEncoder(name, value) { }

};


/**
 * Encodes Date/time into acceptable W3C format.
 * Example of format: "Sun, 05 May 2013 19:51:06 PDT".
 * Defaults to current time, but can be initialized with any epoch time
 * in seconds.
 */
class DllExport TimestampEncoder : public HttpParamEncoder
{
public:
    /** Construct a TimestampEncoder.
     *  @param timeSecs Number of seconds since epoch to set this timestamp to.
     *                  Defaults to current date/time.
     */
    TimestampEncoder(time_t timeSecs = time(0));
    /** Construct a TimestampEncoder.
     *  @param refresh If true, regenerate timestamp get time toString() is called.
     */
    TimestampEncoder(bool refresh);

    virtual std::string toString();
private:
    time_t timeSecs_;
    bool refresh_;
};


/**
 * Class to handle HTTP protocol over a socket connection.
 */
class DllExport HttpStream : public Socket
{
public:
    HttpStream();
	HttpStream(const char* lpszFileName, UINT uOpenFlags);
	HttpStream(SocketAddr& rSockAddr, UINT uOpenFlags);

    virtual ~HttpStream();

    UINT get(const std::string& uri, char* buffer, UINT uCount);
    UINT head(const std::string& uri);
    UINT post(const std::string& uri, char* message, char* buffer, UINT uCount);
    UINT put(const std::string& uri, char* buffer);
    UINT deleter(const std::string& uri);

    void addHeader(const std::string& header, const std::string& value);
    void addHeader(const std::string& header, HttpParamEncoder* encoder,
                   const std::string& value = "");
    void clearHeaders(void);
    void expandHeaders(std::string& str);
    void loadDefaultHeaders(void);

protected:
    typedef std::map<std::string, HttpParamEncoder*> HeaderMap;

    HeaderMap headers;

private:
    HttpStream(const HttpStream&);	// disable copy constructor
    HttpStream& operator=(const HttpStream& rSource);	// disable assignment operator
};

}  // namespace sockstr

#endif // _HTTPSTREAM_H_INCLUDED_
