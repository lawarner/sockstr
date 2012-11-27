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
#include <sstream>
#include <vector>
#include "Condition.h"
#include "Params.h"
#include "Parser.h"
using namespace ipctest;
using namespace std;

const char* opName[] = { "==", "!=", "<", "<=", ">", ">=", "~==", "!~==" };

static void outputVector(const vector<string>& vCond)
{
    vector<string>::const_iterator it(vCond.begin());
    for ( ; it != vCond.end(); ++it)
    {
        string scond = *it;
        cout << scond << " ";
    }
    cout << "." << endl;
}


Condition* Condition::createCondition(const std::string& strCond)
{
    if (strCond.empty())
        return new ConditionFixed(false);

    vector<string> vCond;
    Parser::splitDelimitedTokens(strCond, vCond, "()\",");

    return createCondition(vCond);
}

Condition* Condition::createCondition(vector<string>& vCond)
{
    cout << "Condition tokens:" << endl;
    outputVector(vCond);
    
    if (vCond.empty())
        return 0;

    if (vCond[0] == "(")
    {
        vector<string> vNested(extractNested(vCond));
        cout << "nest start." << endl;
        Condition* condNested = createCondition(vNested);
        cout << "nest end." << endl;
        if (vCond.empty())
            return condNested;
    }

    if (vCond[0] == "false")
        return new ConditionFixed(false);
    else if (vCond[0] == "true")
        return new ConditionFixed(true);

    if (vCond[0] == "and")
    {
        vector<string> vNested(extractNested(vCond));
        cout << "-and nested: ";
        outputVector(vNested);
        if (vNested.size() == 3)	// simple case
        {
            Condition* left = new ConditionFixed(vNested[0]);
            Condition* right = new ConditionFixed(vNested[2]);
            Condition* condAnd = new ConditionAnd(*left, *right);

            cout << "Created and condition: " << condAnd->toString() << endl;
            return condAnd;
        }
    }

    return 0;
}


vector<string> Condition::extractNested(vector<string> vCond)
{
    vector<string> vNested;

    int level = 0;
    vector<string>::iterator it(vCond.begin());
    if (*it != "(") ++it;
    for (++it ; it != vCond.end(); ++it)
    {
        string scond = *it;
        if (scond == "(")
            level++;
        else if (scond == ")")
        {
            if (level == 0)
                break;
            else
                level--;
        }
        vNested.push_back(scond);
    }

    vCond.erase(vCond.begin(), it);
    return vNested;
}

bool Condition::stringToBool(const string& str)
{
    bool val = false;

    istringstream is(str);
    is >> boolalpha >> val;

    return val;
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

