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

#include <sys/types.h>
#include <regex.h>
#include <iostream>
#include <sstream>
#include <vector>

#include "Condition.h"
#include "Params.h"
#include "Parser.h"
#include "RunContext.h"
using namespace ipctest;
using namespace std;

static const char* opName[] = { "operr", "==", "!=", "<", "<=", ">", ">=", "~==", "!~==" };

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
        if (!vCond.empty())
        {
            cout << "Ignore tokens after nest: ";
            outputVector(vCond);
        }
        return condNested;
    }

    if (vCond[0] == "false")
        return new ConditionFixed(false);
    else if (vCond[0] == "true")
        return new ConditionFixed(true);

    Condition* retcond = 0;
    if (vCond[0] == "and" || vCond[0] == "or")
    {
        bool isAnd = (vCond[0] == "and") ? true : false;

        cout << " -" << vCond[0] << " nested: ";
        vector<string> vNested(extractNested(vCond));
        outputVector(vNested);
        if (vNested.size() < 3)
        {
            cout << "Error, not enough parameters" << endl;
        }
        else
        {
            vector<string> vParam(extractParam(vNested));
            Condition* left  = createCondition(vParam);
            vParam = extractParam(vNested);
            Condition* right = createCondition(vParam);
            if (left && right)
            {
                if (isAnd)
                    retcond = new ConditionAnd(*left, *right);
                else
                    retcond = new ConditionOr(*left, *right);
            }
        }
    }
    else if (vCond[0] == "paramvalue")
    {
        vector<string> vNested(extractNested(vCond));
        if (vNested.size() != 5)	// includes 2 commas
        {
            cout << "Error: paramvalue needs exactly 3 parameters" << endl;
        }
        else if (vNested[1] != "," || vNested[3] != ",")
        {
            cout << "Error: malformed statement, missing commas." << endl;
        }
        else
        {
            ComparisionOp cop = getOp(vNested[2]);
            if (cop == OpInvalid)
                cout << "Error: invalid condition - " << vNested[2] << endl;
            else
                retcond = new ConditionParamValue(vNested[0], cop, vNested[4]);
        }
    }

    if (retcond)
        cout << "Created condition: " << retcond->toString() << endl;

    return retcond;
}


vector<string> Condition::extractNested(vector<string>& vCond)
{
    vector<string> vNested;
    if (vCond.size() < 2)
        return vNested;

    int level = 0;
    vector<string>::iterator it(vCond.begin());
    if (*it != "(") ++it;	// skip the word in front of nested expression
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

vector<string> Condition::extractParam(vector<string>& vCond)
{
    vector<string> vNested;

    int level = 0;
    vector<string>::iterator it(vCond.begin());
    for ( ; it != vCond.end(); ++it)
    {
        string scond = *it;
        if (scond == "," && level == 0)
        {
            ++it;
            break;
        }
        else if (scond == "(")
            level++;
        else if (scond == ")")
        {
            if (level > 0)
                level--;
        }
        vNested.push_back(scond);
    }

    vCond.erase(vCond.begin(), it);
    return vNested;
}


ComparisionOp Condition::getOp(const string& str)
{
    for (unsigned int ix = 1; ix < (sizeof(opName) / sizeof(opName[0])); ix++)
    {
        if (str == opName[ix])
            return ComparisionOp(ix);
    }

    return OpInvalid;
}


bool Condition::stringToBool(const string& str)
{
    bool val = false;

    istringstream is(str);
    is >> boolalpha >> val;

    return val;
}


// ParamValue
ConditionParamValue::ConditionParamValue(const std::string& name,
                                         ComparisionOp op, const std::string& val)
    : name_(name)
    , value_(val)
    , oper_(op)
{

}

bool ConditionParamValue::regexMatch(const std::string& str) const
{
    regex_t re;
    if (regcomp(&re, value_.c_str(), REG_EXTENDED | REG_NOSUB))
    {
        cout << "Error: cannot compile regex: " << value_ << endl;
        return false;
    }

    if (regexec(&re, str.c_str(), 0, 0, 0))
    {
        cout << "Did not match regex" << endl;
        regfree(&re);
        return false;
    }

    regfree(&re);
    return true;
}

bool ConditionParamValue::operator() (RunContext& context)
{
    bool ret = false;

#if 1
    std::string pvalue = context.getFieldValue(name_);
#else
    if (!params_)
        return ret;

    std::string pvalue = params_->get(name_);
#endif

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
        ret = regexMatch(pvalue);
        break;
    case OpNotMatches:
        ret = !regexMatch(pvalue);
        break;
    default:
        std::cout << "Error: invalid operator value=" << oper_ << std::endl;
    }

    cout << " comparing " << pvalue << opName[oper_] << " to " << value_ 
         << " returns " << boolalpha << ret << endl;

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

bool ConditionAnd::operator() (RunContext& context)
{
    return left_(context) && right_(context);
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

bool ConditionOr::operator() (RunContext& context)
{
    return left_(context) || right_(context);
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

