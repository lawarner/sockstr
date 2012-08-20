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


namespace ipctest
{

//
// MACRO DEFINITIONS
//
#ifndef DllExport
#define DllExport
#endif

//
// FORWARD CLASS DECLARATIONS
//
class Command;

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
     * @param eHist An enumeration list of history commands.  The first
     * element of the list is the current command. <p>
     * The caller will simply step through the enumeration
     * list, oblivious to any changes that the command may
     * have made to the enumeration list.
     * @return The enumeration list is returned.  For normal commands,
     * this will be the unaltered list that was passed in
     * as a method parameter.  Complex commands such as
     * If and Loop will return modified copies of the
     * enumeration list in order to modify execution.
     */
    virtual CommandIterator& execute(CommandIterator& cmds) = 0;


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

/** This command does nothing. */
class CommandNoop : public Command
{
public:
    CommandNoop()
        : Command("Noop") { }

    virtual CommandIterator& execute(CommandIterator& cmds) { return cmds; }
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

    virtual CommandIterator& execute(CommandIterator& cmds);

private:
    std::string functionName_;
    CommandList commands_;
};

}  // namespace ipctest

#endif
