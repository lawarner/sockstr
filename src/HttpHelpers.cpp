/*
   Copyright (C) 2012 - 2023
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

// HttpHelpers.cpp
//

#include <sockstr/HttpHelpers.h>
#include <sockstr/Socket.h>
#include <sstream>
#include <stdio.h>
#include <time.h>
using namespace sockstr;

const std::string HttpStatus::HTTP_HEADER = HTTP_VERSION " ";
std::map<int, std::string> HttpStatus::statusNames_;

CompoundEncoder::CompoundEncoder(const char* separator)
    : separator_(separator)
    , owned_(true)
{

}

CompoundEncoder::CompoundEncoder(const CompoundEncoder& other, const char* separator)
    : separator_(separator)
    , owned_(false)
{
    encoders_ = other.encoders_;
}

CompoundEncoder::~CompoundEncoder() {
    if (owned_) {
        std::vector<HttpParamEncoder*>::iterator it;
        for (it = encoders_.begin(); it != encoders_.end(); ++it) {
            delete *it;
        }
    }
}

std::string CompoundEncoder::toString() {
    std::string str;
    std::vector<HttpParamEncoder*>::iterator it;
    for (it = encoders_.begin(); it != encoders_.end(); ++it) {
        std::string name = (*it)->getName();
        if (name.empty()) {
            str += (*it)->toString();
        } else {
            str += (*it)->getName() + "=\"" + (*it)->toString() + "\"";
        }
        if (*it != encoders_.back()) {
            str += separator_;
        }
    }
    return str;
}

void CompoundEncoder::addElement(HttpParamEncoder* encoder) {
    encoders_.push_back(encoder);
}


HostnameEncoder::HostnameEncoder(Socket& socket)
    : socket_(socket) {}

std::string HostnameEncoder::toString() {
    std::string strhost((const char *) socket_);
    if (strhost.empty()) {
        //TODO show dot address
        strhost = "(Unknown)";
    } else {
        std::size_t colon = strhost.find_last_of(':');
        strhost = strhost.substr(0, colon);
    }
    return strhost;
}


TimestampEncoder::TimestampEncoder(time_t timeSecs, DateTimeFormat format)
    : timeSecs_(timeSecs)
    , refresh_(false)
    , format_(format)
{

}

TimestampEncoder::TimestampEncoder(bool refresh, DateTimeFormat format)
    : timeSecs_(time(0))
    , refresh_(refresh)
    , format_(format)
{

}

std::string TimestampEncoder::toString() {
    char outstr[200];
    struct tm* tmp;
    if (refresh_)
        timeSecs_ = time(0);
    tmp = localtime(&timeSecs_);

    if (format_ == DateTimeRfc822)
        strftime(outstr, sizeof(outstr), "%a, %d %b %Y %T %Z", tmp);
    else
        snprintf(outstr, sizeof(outstr), "%ld", timeSecs_);

    return std::string(outstr);
}


std::string UrlParameterEncoder::toString() {
    std::string outstr(urlEncode(getName()) + "=" + urlEncode(HttpParamEncoder::toString()));
    return outstr;
}

#define VALID_IN_URL "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.-_~"

std::string UrlParameterEncoder::urlEncode(const std::string& inStr) {
    std::ostringstream oss;
    // Space can be encoded as either %20 or +
    size_t lastPos = 0;
    size_t pos = inStr.find_first_not_of(VALID_IN_URL);
    while (pos != std::string::npos) {
        if (lastPos != pos) {
            oss << inStr.substr(lastPos, pos - lastPos);
        }
        unsigned int ich = inStr[pos];
        oss << (ich < 16 ? "%0" : "%") << std::hex << ich;

        lastPos = pos + 1;
        pos = inStr.find_first_not_of(VALID_IN_URL, lastPos);
    }

    oss << inStr.substr(lastPos);
    return oss.str();
}


HttpStatus::HttpStatus() : status_(200) {}
HttpStatus::HttpStatus(int status) : status_(status) {}
HttpStatus::HttpStatus(const std::string& statusLine)
    : status_(0) {
    if (statusLine.size() > 3) {
        int st = atoi(statusLine.c_str());
        if (st > 0) {
            status_ = st;
        }
    }
}

void HttpStatus::init_() {
    if (statusNames_.empty()) {
        statusNames_[100] = " Continue";
        statusNames_[200] = " OK";
        statusNames_[400] = " Bad Request";
        statusNames_[401] = " Unauthorized";
        statusNames_[403] = " Forbidden";
        statusNames_[404] = " Not Found";
    }
}

int HttpStatus::getStatus() const {
    return status_;
}

void HttpStatus::setStatus(int status) {
    status_ = status;
}

std::string HttpStatus::statusLine() const {
    std::ostringstream oss;
    oss << HTTP_HEADER << status_ << statusName() << "\r" << std::endl;
    return oss.str();
}

std::string HttpStatus::statusName() const {
    if (statusNames_.find(status_) != statusNames_.end()) {
        return statusNames_[status_];
    }
    return " ";
}
