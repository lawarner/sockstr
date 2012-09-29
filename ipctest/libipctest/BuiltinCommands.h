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

#ifndef _BUILTIN_COMMANDS_H_INCLUDED_
#define _BUILTIN_COMMANDS_H_INCLUDED_
//
//

//
// INCLUDE FILES
//
#include "Command.h"


//
// FORWARD CLASS DECLARATIONS
//
namespace sockstr
{
    class Socket;
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
// some of the basic commands are defined here:

/** Comment command. */
class CommandComment : public Command
{
public:
	CommandComment(const std::string& comment);

    virtual bool execute(RunContext& context);
    virtual std::string toString();
    virtual std::string toXml(int indent);
};

/** Connect command. */
class CommandConnect : public Command
{
public:
    CommandConnect(const std::string& url);

    virtual bool execute(RunContext& context);
    virtual std::string toString();
    virtual std::string toXml(int indent);

    sockstr::Socket* getSocket() const;

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
    virtual std::string toString();
    virtual std::string toXml(int indent);

private:
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
    CommandReceive(Message* msg = 0);

    virtual bool execute(RunContext& context);
    virtual std::string toString();
    virtual std::string toXml(int indent);
};

/** Send command. */
class CommandSend : public Command
{
public:
    CommandSend(Message* msg = 0, void* msgData = 0);

    virtual bool execute(RunContext& context);
    virtual std::string toString();
    virtual std::string toXml(int indent);
};

}  // namespace ipctest

#endif
