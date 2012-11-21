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

// Condition.cpp
//

#include <iostream>
#include <vector>
#include "Condition.h"
#include "Params.h"
#include "Parser.h"
using namespace ipctest;
using namespace std;

const char* opName[] = { "==", "!=", "<", "<=", ">", ">=", "~==", "!~==" };


Condition* Condition::createCondition(const std::string& strCond)
{
    if (strCond.empty())
        return new ConditionFixed(false);

    vector<string> vCond;
    Parser::splitDelimitedTokens(strCond, vCond, "()\",");

    cout << "Condition:" << endl;
    vector<string>::iterator it(vCond.begin());
    for ( ; it != vCond.end(); ++it)
    {
        string scond = *it;
//        if (scond == "param")
        cout << scond << " ";
    }
    cout << "." << endl;
    
    return 0;
}


ConditionParamValue::ConditionParamValue(const std::string& name,
                                          Operator op, const std::string& val)
    : name_(name)
    , value_(val)
    , oper_(op)
    , params_(0)
{

}


void ConditionParamValue::setParams(Params* params)
{
    params_ = params;
}

bool ConditionParamValue::operator() ()
{
    bool ret = false;

    if (!params_)
        return ret;

    std::string pvalue = params_->get(name_);

    switch (oper_)
    {
    case OpEqual:
        ret = (pvalue == value_);
        break;
    case OpNotEqual:
        ret = (pvalue != value_);
        break;
    case OpLessThan:
        ret = (pvalue < value_);
        break;
    case OpLessThanEqual:
        ret = (pvalue <= value_);
        break;
    case OpGreaterThan:
        ret = (pvalue > value_);
        break;
    case OpGreaterThanEqual:
        ret = (pvalue >= value_);
        break;
    case OpMatches:
        break;
    case OpNotMatches:
        break;
    default:
        std::cout << "Error: invalid operator value=" << oper_ << std::endl;
    }

    return ret;
}

std::string ConditionParamValue::toString()
{
    std::string str = name_ + opName[oper_] + "\"" + value_ + "\"";
    return str;
}

std::string ConditionParamValue::toXml(int indent)
{
    std::string str(indent, ' ');
    str += "<Condition type=\"paramvalue\" name=\"" + name_ + " op=\"" + opName[oper_]
        + "\" value=\"" + value_ + "\"/>\n";
    return str;
}

// ------------------------------------------------------
ConditionAnd::ConditionAnd(Condition& left, Condition& right)
    : ConditionBinary(left, right)
{

}

bool ConditionAnd::operator() ()
{
    return left_() && right_();
}


std::string ConditionAnd::toString()
{
    return left_.toString() + " and " + right_.toString();
}

std::string ConditionAnd::toXml(int indent)
{
    std::string strdent(indent, ' ');
    std::string str = strdent + "<Condition type=\"and\">\n" 
        + left_.toXml(indent+4) + right_.toXml(indent+4)
        + strdent + "</Condition>\n";

    return str;
}

// ------------------------------------------------------
ConditionOr::ConditionOr(Condition& left, Condition& right)
    : ConditionBinary(left, right)
{

}

bool ConditionOr::operator() ()
{
    return left_() || right_();
}

std::string ConditionOr::toString()
{
    return left_.toString() + " or " + right_.toString();
}

std::string ConditionOr::toXml(int indent)
{
    std::string strdent(indent, ' ');
    std::string str = strdent + "<Condition type=\"or\">\n" 
        + left_.toXml(indent+4) + right_.toXml(indent+4)
        + strdent + "</Condition>\n";

    return str;
}

