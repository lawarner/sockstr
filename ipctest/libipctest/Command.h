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

#ifndef _COMMAND_H_INCLUDED_
#define _COMMAND_H_INCLUDED_
//
//

//
// INCLUDE FILES
//
#include <map>
#include <string>
#include <vector>

#include "Params.h"

//
// FORWARD CLASS DECLARATIONS
//
namespace ipctest
{
    class Message;
    class RunContext;
}


namespace ipctest
{

//
// MACRO DEFINITIONS
//
#ifndef DllExport
#define DllExport
#endif

//
// CLASS DEFINITIONS
//
// template<typename DATATYPE>
class Command
{
public:
    static Command* createCommand(const std::string& cmdName, 
                                  Params* params, Message* msg = 0);
    static void registerCommand(Command* cmd);

    static int bumpLevel(int incr = 0);

    virtual Command* createCommand(Params* params, Message* msg = 0) = 0;

   /** Run the command in a manner specific to the sub-class.
     * @param context The run context which contains an iterator through
     * a set of commands.  The first element of the list is the current command. <p>
     * The caller will simply step through the enumeration
     * list, oblivious to any changes that the command may
     * have made to the iterator. <p>
     * Complex commands such as If and Loop will modify the iterator
     *  within the run context in order to modify execution.
     * @return true if command succeeded.
     */
    virtual bool execute(RunContext& context) = 0;

    int  getLevel() { return level_; }
    void setLevel(int level) { level_ = level; }
    std::string getName() { return commandName_; }
    std::string getParam(const std::string& name)
    {
        std::string val;
        if (params_)
            params_->get(name, val);
        return val;
    }
    void* getData() { return data_; }
    void  setData(void* data) { data_ = data; }
    Message* getMessage() const { return message_; }
    void setMessage(Message* msg) { message_ = msg; }
    Params* getParams() { return params_; }
    void setParams(Params* params) { params_ = params; }
    std::string getXmlPart(int indent, bool header);
    std::string getXmlPart(int indent, std::string& strAttr, bool header);

    virtual void initParams() { }

    virtual std::string toString() { return commandName_; }
    virtual std::string toXml(int indent);

protected:
    Command(const std::string& cname, Params* params = 0, Message* msg = 0) 
        : params_(params ? params : new Params)
        , message_(msg)
        , commandName_(cname)
        , data_(0), delay_(0)
        , level_(currLevel_)  { }
    Command(const std::string& cname, 
            const std::string& keyname,
            const std::string& keyvalue = std::string(),
            void* data = 0, int delay = 0)
        : params_(new Params), message_(0), commandName_(cname)
        , data_(data), delay_(delay) 
        , level_(currLevel_)
    {
        params_->set(keyname, keyvalue);
    }
    Command(const std::string& cname, 
            const std::string& keyname,
            int keyvalue = 0,
            void* data = 0, int delay = 0)
        : params_(new Params), message_(0), commandName_(cname)
        , data_(data), delay_(delay) 
        , level_(currLevel_)
    {
        params_->set(keyname, keyvalue);
    }
    Command(const std::string& cname, void* data, int delay = 0)
        : params_(new Params), message_(0)
        , commandName_(cname), data_(data), delay_(delay)
        , level_(currLevel_)
    {  }

    int getDelay() const { return delay_; }
    void setDelay(int delay) { delay_ = delay; }

private:
    Command(const Command&);	// disable copy constructor
    Command& operator=(const Command& rSource);	// disable assignment operator

protected:
    Params* params_;
    Message* message_;
    std::string commandName_;	// Name of command
    void* data_;	// any message related to command
    int delay_;		// delay before executing (for realtime playback)
    int level_;		// nesting level of this command

    static Message* emptyMessage_;
    static int currLevel_;		// nesting level while running commands

private:
    static std::map<std::string, Command*> sCommands_;
};


//
// TYPE DEFINITIONS
//
// typedef BasicCommand<void *> Command;
typedef std::vector<Command *> CommandList;
typedef CommandList::iterator CommandIterator;


}  // namespace ipctest

#endif
