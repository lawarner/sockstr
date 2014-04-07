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

#ifndef _HTTPHELPERS_H_INCLUDED_
#define _HTTPHELPERS_H_INCLUDED_
//
//

//
// INCLUDE FILES
//
#include <map>
#include <string>
#include <vector>


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
    void sortElements() { } //TODO: implement

protected:
    std::vector<HttpParamEncoder*> encoders_;
private:
    const char* separator_;
};


/**
 * This encoder contains a constant string value.  It can emit just a value
 * but it is likely most useful as a name/value pair.
 */
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
    /** Type of output for the date time encoder. */
    enum DateTimeFormat
    {
        DateTimeRfc822,		//!< Output in RFC-822 format
        DateTimeRaw			//!< Output as number of seconds since epoch
    };

    /** Construct a TimestampEncoder.
     *  @param timeSecs Number of seconds since epoch to set this timestamp to.
     *                  Defaults to current date/time.
     *  @param format  The output format of the date.  Either DateTimeRfc822
     *                 or DateTimeRaw.
     */
    TimestampEncoder(time_t timeSecs = time(0), DateTimeFormat format = DateTimeRfc822);
    /** Construct a TimestampEncoder.
     *  @param refresh If true, regenerate timestamp from current time when toString()
     *                 is called.
     *  @param format  The output format of the date.  Either DateTimeRfc822
     *                 or DateTimeRaw.
     */
    TimestampEncoder(bool refresh, DateTimeFormat format = DateTimeRfc822);

    virtual std::string toString();
private:
    time_t timeSecs_;
    bool refresh_;

    DateTimeFormat format_;
};


class DllExport HttpStatus
{
public:
    HttpStatus();
    HttpStatus(int status);
    HttpStatus(const std::string& statusLine);

    int getStatus() const;
    void setStatus(int status);
    std::string statusLine() const;
    std::string statusName() const;

private:
    void init_();

public:
    static const std::string HTTP_HEADER;

private:
    static std::map<int, std::string> statusNames_;

    int status_;
};

}  // namespace sockstr

#endif // _HTTPHELPERS_H_INCLUDED_
