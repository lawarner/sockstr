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

// Params.cpp
//
#include <stdlib.h>
#include <sstream>

#include "Params.h"
using namespace ipctest;


Params::Params()
{

}

Params::~Params()
{
    env_.clear();
}


bool Params::get(const std::string& name, int& value)
{
    std::string str;
    if (!get(name, str))
        return false;

    value = atoi(str.c_str());
    return true;
}


bool Params::get(const std::string& name, std::string& value)
{
    std::map<std::string, std::string>::iterator it;

    it = env_.find(name);
    if (it == env_.end())
        return false;

	value = it->second;
    return true;
}

bool Params::set(const std::string& name, int value)
{
    std::stringstream ss;
    ss << value;
    return set(name, ss.str());
}

bool Params::set(const std::string& name, const std::string& value)
{
    env_[name] = value;
    return true;
}


