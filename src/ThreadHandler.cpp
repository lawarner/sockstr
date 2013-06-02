/*
   Copyright (C) 2012, 2013
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

// ThreadHandler.cpp
//

#include "config.h"
#include <cassert>
#if CONFIG_HAS_PTHREADS
#include <pthread.h>
#endif

#include <sockstr/ThreadHandler.h>
using namespace sockstr;

#ifdef _DEBUG
void* ThreadHandler::m_pLastBuffer = 0;
#endif


bool
ThreadManager::create(THRTYPE_FUNCTION function, void* data, bool start)
{
#if CONFIG_HAS_PTHREADS
    pthread_t thread_id;
    int st = pthread_create(&thread_id, NULL, function, data);
    VERIFY(st==0);
#else
#ifdef WIN32
#ifdef USE_MFC
    VERIFY(AfxBeginThread((AFX_THREADPROC) function, data));
#else
    DWORD thread_id;
    VERIFY(CreateThread( 
               NULL,            // default security attributes
               0,               // use default stack size  
               function,        // thread function name
               data,            // argument to thread function 
               0,               // use default creation flags 
               &thread_id));	// returns the thread identifier 
#endif
#else
    // multi-threaded mode not implemented for this platform
    VERIFY(0);
#endif
#endif
    return true;
}


bool ThreadManager::_launchThread(THRTYPE_FUNCTION function, void* handler)
{
#if CONFIG_HAS_PTHREADS
    pthread_t thread_id;
    int st = pthread_create(&thread_id, NULL, 
                            function,
                            handler);
    return (st == 0);
#else
#ifdef WIN32
#ifdef USE_MFC
    VERIFY(AfxBeginThread((AFX_THREADPROC) function, handler));
#else
    DWORD thread_id;
    VERIFY(CreateThread( 
               NULL,            // default security attributes
               0,               // use default stack size  
               function,        // thread function name
               handler,            // argument to thread function 
               0,               // use default creation flags 
               &thread_id));	// returns the thread identifier 
#endif
#else
    // multi-threaded mode not implemented for this platform
    VERIFY(0);
#endif
#endif


    // TODO: keep a list of all handlers so they can be managed and deleted

    return true;
}



