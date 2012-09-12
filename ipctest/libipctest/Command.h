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
#include <string>
#include <vector>

#include "Params.h"

//
// FORWARD CLASS DECLARATIONS
//
namespace ipctest
{
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

    std::string getName() { return commandName_; }
    std::string getParam(const std::string& name)
    {
        std::string val;
        if (params_)
            params_->get(name, val);
        return val;
    }
    virtual std::string toString() { return commandName_; }

protected:
	Command(Params* params = 0) : params_(params) { }
    Command(const std::string& cname, 
            const std::string& msgname = std::string(),
            void* data = 0, int delay = 0)
        : commandName_(cname), messageName_(msgname), data_(data), delay_(delay) {  }
    Command(const std::string& cname, 
            Params* params,
            void* data = 0, int delay = 0)
        : params_(params), commandName_(cname), data_(data), delay_(delay) {  }

    void* getData() const { return data_; }
    int getDelay() const { return delay_; }
    void setDelay(int delay) { delay_ = delay; }
    Params* getParams() { return params_; }
    void setParams(Params* params) { params_ = params; }

private:
    Command(const Command&);	// disable copy constructor
    Command& operator=(const Command& rSource);	// disable assignment operator

protected:
    Params* params_;
    std::string commandName_;	// Name of command
    std::string messageName_;	// Name of message
    void* data_;	// any message related to command
    int delay_;		// delay before executing (for realtime playback)

};


//
// TYPE DEFINITIONS
//
// typedef BasicCommand<void *> Command;
typedef std::vector<Command *> CommandList;
typedef CommandList::iterator CommandIterator;


}  // namespace ipctest

#endif
