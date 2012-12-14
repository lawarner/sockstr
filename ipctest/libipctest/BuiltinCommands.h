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
class Condition;

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
	CommandComment(Params* params, Message* msg = 0);

    virtual Command* createCommand(Params* params, Message* msg = 0);
    virtual bool execute(RunContext& context);
    virtual std::string toString();
    virtual std::string toXml(int indent);
};

/** Connect command. */
class CommandConnect : public Command
{
public:
    CommandConnect(const std::string& url);
	CommandConnect(Params* params, Message* msg = 0);

    virtual Command* createCommand(Params* params, Message* msg = 0);
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
	CommandDisconnect(Params* params, Message* msg = 0);

    virtual Command* createCommand(Params* params, Message* msg = 0);
    virtual bool execute(RunContext& context);
};

/** This command executes an external system command and captures its
 *  output.
 */
class CommandExec : public Command
{
public:
    CommandExec(const std::string& cmdLine, bool waitFor = false, 
                void* data = 0, int delay = 0);
    CommandExec(Params* params, Message* msg = 0);

    virtual Command* createCommand(Params* params, Message* msg = 0);
    virtual bool execute(RunContext& context);
    virtual void initParams();
    virtual std::string toString();
    virtual std::string toXml(int indent);

private:
    std::string commandLine_;
    bool waitFor_;
    int pid_;
};

/** This command is a container that holds a sequence of
 *  commands.  It is useful for executing (or running) a 
 *  sequence of commands, as in a test case.
 */
class CommandFunction : public Command
{
public:
    CommandFunction(const std::string& funcName, void* data = 0, int delay = 0);
	CommandFunction(Params* params, Message* msg = 0);
    void addCommand(Command* cmd);

    virtual Command* createCommand(Params* params, Message* msg = 0);
    virtual bool execute(RunContext& context);
    virtual std::string toString();
    virtual std::string toXml(int indent);

private:
    CommandList commands_;
};

/** This command tests its condition and if true executes the contained
 *  (nested) commands.  Commands following the "If" that are higher level
 *  are executed.  It is possible to encounter an "Else" command at the same
 *  level as the "If".  In this case, an standard if/else is formed.
 */
class CommandIf : public Command
{
public:
    CommandIf(Condition* condition, Command* cmd = 0);
	CommandIf(Params* params, Message* msg = 0);
    void addCondition(Condition* condition);

    virtual Command* createCommand(Params* params, Message* msg = 0);
    virtual bool execute(RunContext& context);
    virtual std::string toString();
    virtual std::string toXml(int indent);

private:
    Condition* condition_;
    CommandList commands_;
};

class CommandElse : public Command
{
public:
    CommandElse(Command* cmd = 0);
	CommandElse(Params* params, Message* msg = 0);

    virtual Command* createCommand(Params* params, Message* msg = 0);
    virtual bool execute(RunContext& context);
    virtual std::string toXml(int indent);

private:
    CommandList commands_;
};

/** This command loops through other commands.  In the simpliest
 *  form this command loops a fixed number of iterations of one command.
 *  The command can of course be a function containing other commands.
 */
class CommandLoop : public Command
{
public:
    CommandLoop(int iters, Command* cmd = 0);
	CommandLoop(Params* params, Message* msg = 0);
    void addCommand(Command* cmd);

    virtual Command* createCommand(Params* params, Message* msg = 0);
    virtual bool execute(RunContext& context);
    virtual std::string toString();
    virtual std::string toXml(int indent);

private:
    int curr_;
    CommandList commands_;
};

/** This command does nothing. */
class CommandNoop : public Command
{
public:
    CommandNoop()
        : Command("Noop", (void *)0, 0) { }
	CommandNoop(Params* params, Message* msg = 0)
        : Command("Noop", params, msg) { }

    virtual Command* createCommand(Params* params, Message* msg = 0)
    {
        return new CommandNoop(params, msg);
    }
    virtual bool execute(RunContext& context)  { return true; }
};

/** Receive command. */
class CommandReceive : public Command
{
public:
    CommandReceive(Message* msg = 0);
	CommandReceive(Params* params, Message* msg = 0);

    virtual Command* createCommand(Params* params, Message* msg = 0);
    virtual bool execute(RunContext& context);
    virtual std::string toString();
    virtual std::string toXml(int indent);
};

/** Send command. */
class CommandSend : public Command
{
public:
    CommandSend(Message* msg = 0, void* msgData = 0);
	CommandSend(Params* params, Message* msg = 0);

    virtual Command* createCommand(Params* params, Message* msg = 0);
    virtual bool execute(RunContext& context);
    virtual std::string toString();
    virtual std::string toXml(int indent);
};


/** This command tests its condition and if true executes the contained
 *  (nested) commands.  Commands following the "While" that are higher level
 *  are executed.  After execution, this command repeats as long as the
 *  condition holds true.
 */
class CommandWhile : public Command
{
public:
    CommandWhile(Condition* condition, Command* cmd = 0);
	CommandWhile(Params* params, Message* msg = 0);
    void addCondition(Condition* condition);

    virtual Command* createCommand(Params* params, Message* msg = 0);
    virtual bool execute(RunContext& context);
    virtual std::string toString();
    virtual std::string toXml(int indent);

private:
    Condition* condition_;
    CommandList commands_;
};

}  // namespace ipctest

#endif
