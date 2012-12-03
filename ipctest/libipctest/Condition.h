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

#ifndef _CONDITION_H_INCLUDED_
#define _CONDITION_H_INCLUDED_
//

//
// INCLUDE FILES
//
#include <string>
#include <vector>


namespace ipctest
{
//
// FORWARD CLASS DECLARATIONS
//
class Params;
class RunContext;

//
// MACRO DEFINITIONS
//
#ifndef DllExport
#define DllExport
#endif

//
// TYPE DEFINITIONS
//
enum ComparisionOp
{
    OpInvalid,
    OpEqual,
    OpNotEqual,
    OpLessThan,
    OpLessThanEqual,
    OpGreaterThan,
    OpGreaterThanEqual,
    OpMatches,
    OpNotMatches
};

//
// CLASS DEFINITIONS
//
class Condition
{
public:
    Condition()
        : params_(0) { }
    /*! Factory method */
    static Condition* createCondition(const std::string& strcond);
    static Condition* createCondition(std::vector<std::string>& vCond);
    /*!
     *  Extract a nested expression within parentheses and return as its own 
     *  expression.  The nested portion is deleted from the original
     *  expression (string vector).
     */
    static std::vector<std::string> extractNested(std::vector<std::string>& vCond);
    /*! Extract an expression delimited by commas, considering nesting. */
    static std::vector<std::string> extractParam(std::vector<std::string>& vCond);
    static ComparisionOp getOp(const std::string& str);
    static bool stringToBool(const std::string& str);

    void setParams(Params* params) { params_ = params; }

    virtual ~Condition() { }

    virtual bool operator() (RunContext& context) = 0;

    virtual std::string toString() = 0;
    virtual std::string toXml(int indent) = 0;

protected:
    Params* params_;

private:
    Condition& operator=(const Condition& rSource);	// disable assignment operator
    Condition(const Condition& other);	// copy constructor
};


class ConditionSingle : public Condition
{

};

class ConditionBinary : public Condition
{
protected:
    ConditionBinary(Condition& left, Condition& right)
        : left_(left), right_(right) {  }

    Condition& left_;
    Condition& right_;
};


class ConditionFixed : public ConditionSingle
{
public:
    ConditionFixed(bool cond)
        : condition_(cond) {  }
    ConditionFixed(const std::string& cond)
        : condition_(stringToBool(cond)) {  }

    virtual bool operator() (RunContext& context) { return condition_; }

    virtual std::string toString() { return (condition_ ? "true" : "false"); }
    virtual std::string toXml(int indent)
    {
        std::string str(indent, ' ');
        str += "<Condition type=\"fixed\" value=\"" + toString() + "\"/>\n";
        return str;
    }

private:
    bool condition_;
};


class ConditionParamValue : public ConditionSingle
{
public:
    ConditionParamValue(const std::string& name, ComparisionOp op, const std::string& val);

    bool regexMatch(const std::string& str) const;

    virtual bool operator() (RunContext& context);

    virtual std::string toString();
    virtual std::string toXml(int indent);

private:
    std::string name_;
    std::string value_;
    ComparisionOp oper_;
};


class ConditionAnd : public ConditionBinary
{
public:
    ConditionAnd(Condition& left, Condition& right);
    virtual bool operator() (RunContext& context);

    virtual std::string toString();
    virtual std::string toXml(int indent);
};


class ConditionOr : public ConditionBinary
{
public:
    ConditionOr(Condition& left, Condition& right);
    virtual bool operator() (RunContext& context);

    virtual std::string toString();
    virtual std::string toXml(int indent);
};


}  // namespace ipctest

#endif
