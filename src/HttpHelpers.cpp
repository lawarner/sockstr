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

// HttpHelpers.cpp
//

#include <sockstr/HttpHelpers.h>
#include <sstream>
#include <stdio.h>
#include <time.h>
using namespace sockstr;
using namespace std;

const std::string HttpStatus::HTTP_HEADER = "HTTP/1.1 ";
std::map<int, std::string> HttpStatus::statusNames_;

CompoundEncoder::CompoundEncoder(const char* separator)
    : separator_(separator)
{

}

CompoundEncoder::~CompoundEncoder()
{
    std::vector<HttpParamEncoder*>::iterator it;
    for (it = encoders_.begin(); it != encoders_.end(); ++it)
    {
        delete *it;
    }
}


std::string CompoundEncoder::toString()
{
    std::string str;
    std::vector<HttpParamEncoder*>::iterator it;
    for (it = encoders_.begin(); it != encoders_.end(); ++it)
    {
        std::string name = (*it)->getName();
        if (name.empty())
            str += (*it)->toString();
        else
            str += (*it)->getName() + "=\"" + (*it)->toString() + "\"";
        if (*it != encoders_.back())
            str += separator_;
    }
    return str;
}

void CompoundEncoder::addElement(HttpParamEncoder* encoder)
{
    encoders_.push_back(encoder);
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

std::string TimestampEncoder::toString()
{
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


HttpStatus::HttpStatus() : status_(200) { }
HttpStatus::HttpStatus(int status) : status_(status) { }
HttpStatus::HttpStatus(const std::string& statusLine)
{

}

void HttpStatus::init_()
{
    if (statusNames_.empty())
    {
        statusNames_[100] = *new string(" Continue");
        statusNames_[200] = *new string(" OK");
        statusNames_[400] = " Bad Request";
        statusNames_[401] = " Unauthorized";
        statusNames_[403] = " Forbidden";
        statusNames_[404] = " Not Found";
    }
}

int HttpStatus::getStatus() const
{
    return status_;
}

void HttpStatus::setStatus(int status)
{
    status_ = status;
}

std::string HttpStatus::statusLine() const
{
    std::ostringstream oss;
    oss << HTTP_HEADER << status_ << statusName() << "\r" << std::endl;
    return oss.str();
}

std::string HttpStatus::statusName() const
{
    if (statusNames_.find(status_) != statusNames_.end())
    {
        return statusNames_[status_];
    }

    return " ";
}
