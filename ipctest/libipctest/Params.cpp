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
#include <iostream>
#include <sstream>

#include "Params.h"
using namespace ipctest;


Params::Params()
{

}

Params::Params(const Params& other)
{
    env_ = other.env_;
}

Params::~Params()
{
    clear();
}


void Params::clear()
{
    env_.clear();
}

bool Params::empty() const
{
    return env_.empty();
}


bool Params::get(const std::string& name, int& value) const
{
    std::string str;
    if (!get(name, str))
        return false;

    value = atoi(str.c_str());
    return true;
}

bool Params::get(const std::string& name, bool& value) const
{
    std::string str;
    if (!get(name, str))
        return false;

    if (str=="true" || str=="1")
        value = true;
    else
        value = false;

    return true;
}

bool Params::get(const std::string& name, std::string& value) const
{
    ParamMap::const_iterator it = env_.find(name);
    if (it == env_.end())
        return false;

    ParamValue* pv = it->second;
	value = pv->strValue;
    return true;
}


bool Params::get(const std::string& name, ParamValue*& value) const
{
    ParamMap::const_iterator it = env_.find(name);
    if (it == env_.end())
        return false;

    value = it->second;
    return true;
}


const std::string Params::get(const std::string& name) const
{
    ParamMap::const_iterator it = env_.find(name);
    if (it == env_.end())
        return "";

    ParamValue* pv = it->second;
	return pv->strValue;
}


bool Params::getWidget(const std::string& name, Gtk::Entry*& widget) const
{
    ParamMap::const_iterator it = env_.find(name);
    if (it == env_.end())
        return false;

    ParamValue* pv = it->second;
	widget = pv->widget;
    return true;
}


int Params::loadFromNameValues(const char** nv)
{
    if (!nv) return 0;

    int nr = 0;

    std::string ul("_");
    while (nv[nr*2])
    {
        std::string val(nv[nr*2 + 1]);
        set(ul + nv[nr*2], val);
        nr++;
    }

    return nr;
}


bool Params::set(const std::string& name, int value)
{
    std::stringstream ss;
    ss << value;
    return set(name, ss.str());
}

bool Params::set(const std::string& name, bool value)
{
    std::string str(value ? "true" : "false");
    return set(name, str);
}

bool Params::set(const std::string& name, const std::string& value)
{
    ParamValue* pv;
    ParamMap::iterator it = env_.find(name);
    if (it != env_.end())
    {
        pv = it->second;	// overwrite existing value
        std::cout << "Param " << name << " already exists with value "
                  << pv->strValue << std::endl
                  << " - redefining to " << value << std::endl;
    }
    else
    {
        pv = new ParamValue;
        pv->widget = 0;
        env_[name] = pv;
    }
    pv->strValue = value;
    
    return true;
}


bool Params::set(const std::string& name, ParamValue* value)
{
    env_[name] = value;
    return true;
}

bool Params::setWidget(const std::string& name, Gtk::Entry* widget)
{
    ParamValue* pv;
    ParamMap::iterator it = env_.find(name);
    if (it != env_.end())
        pv = it->second;
    else
    {
        pv = new ParamValue;
        env_[name] = pv;
    }
    pv->widget = widget;

    return true;
}


const ParamMap& Params::getAllParams() const
{
    return env_;
}


