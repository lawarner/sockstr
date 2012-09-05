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


//
// FORWARD CLASS DECLARATIONS
//
namespace sockstr
{
    class Socket;
}
namespace ipctest
{
    class Command;
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
// TYPE DEFINITIONS
//
typedef std::vector<Command *> CommandList;
typedef CommandList::iterator CommandIterator;

//
// CLASS DEFINITIONS
//
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


protected:
    Command(void) { };
    Command(const std::string& cname, void* data = 0, int delay = 0)
        : commandName_(cname), data_(data), delay_(delay) {  }

private:
    Command(const Command&);	// disable copy constructor
    Command& operator=(const Command& rSource);	// disable assignment operator

protected:
    std::string commandName_;	// Name of command
    void* data_;	// any message related to command
    int delay_;		// delay before executing (for realtime playback)

};


// some of the basic commands are defined here:

/** Comment command. */
class CommandComment : public Command
{
public:
	CommandComment(const std::string& comment)
        : Command("Comment")
    	, comment_(comment)  { }

    virtual bool execute(RunContext& context)  { return true; }

private:
    std::string comment_;
};

/** Connect command. */
class CommandConnect : public Command
{
public:
    CommandConnect(const std::string& url);

    virtual bool execute(RunContext& context);

    sockstr::Socket* getSocket() const;

private:
    std::string url_;
};

/** Connect command. */
class CommandDisconnect : public Command
{
public:
    CommandDisconnect();

    virtual bool execute(RunContext& context);
};

/** This command is a container that holds a sequence of
 *  commands.  It is useful for executing (or running) a 
 *  sequence of commands, as in a test case.
 */
class CommandFunction : public Command
{
public:
    CommandFunction(const std::string& funcName, void* data = 0, int delay = 0);
    void addCommand(Command* cmd);

    virtual bool execute(RunContext& context);

private:
    std::string functionName_;
    CommandList commands_;
};

/** This command does nothing. */
class CommandNoop : public Command
{
public:
    CommandNoop()
        : Command("Noop") { }

    virtual bool execute(RunContext& context)  { return true; }
};

/** Receive command. */
class CommandReceive : public Command
{
public:
    CommandReceive();

    virtual bool execute(RunContext& context);

};

/** Send command. */
class CommandSend : public Command
{
public:
    CommandSend();

    virtual bool execute(RunContext& context);

};

}  // namespace ipctest

#endif
