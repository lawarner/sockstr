/*
   Copyright (C) 2013
   Andy Warner
   This file is part of the sockstr class library.

   The sockstr class library and other related programs such as ipctest is free 
   software; you can redistribute it and/or
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

#ifndef _THREADMANAGER_H_INCLUDED_
#define _THREADMANAGER_H_INCLUDED_
//
//

//
// INCLUDE FILES
//


namespace sockstr
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

//
// TYPE DEFINITIONS
//

//
// CLASS DEFINITIONS
//
/**
 * This class manages the creation and execution of threads.
 * This is a convenience so the rest of the library does not have to worry
 * about threads that are platform specific.
 */
class DllExport ThreadManager
{
public:
    ThreadManager() { }
    virtual ~ThreadManager() { }

    virtual void run() { }

    void setHandler(void* handler) { handler_ = handler; }

private:
    void* handler_;
};

}  // namespace sockstr

#endif // _THREADMANAGER_H_INCLUDED_
