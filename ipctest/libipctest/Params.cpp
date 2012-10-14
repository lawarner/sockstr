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
    clear();
}


void Params::clear()
{
    env_.clear();
}

int Params::loadFromNameValues(const char** nv)
{
    if (!nv) return 0;

    int nr = 0;

    while (nv[nr*2])
    {
        set(nv[nr*2], nv[nr*2 + 1]);
        nr++;
    }

    return nr;
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
    ParamMap::iterator it = env_.find(name);
    if (it == env_.end())
        return false;

    ParamValue* pv = it->second;
	value = pv->strValue;
    return true;
}


bool Params::get(const std::string& name, ParamValue*& value)
{
    ParamMap::iterator it = env_.find(name);
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


bool Params::getWidget(const std::string& name, Gtk::Entry*& widget)
{
    ParamMap::iterator it = env_.find(name);
    if (it == env_.end())
        return false;

    ParamValue* pv = it->second;
	widget = pv->widget;
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
    ParamValue* pv;
    ParamMap::iterator it = env_.find(name);
    if (it != env_.end())
        pv = it->second;
    else
    {
        pv = new ParamValue;
        pv->strValue = value;
        env_[name] = pv;
    }
    
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
        pv->widget = widget;
        env_[name] = pv;
    }

    return true;
}


const ParamMap& Params::getAllParams() const
{
    return env_;
}


