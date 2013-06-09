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
#include <unistd.h>
#if CONFIG_HAS_PTHREADS
#include <pthread.h>
#endif

#include <sockstr/ThreadHandler.h>
using namespace sockstr;

#ifdef _DEBUG
void* ThreadHandler::pLastBuffer = 0;
#endif
std::vector<THRTYPE_ID> ThreadManager::threads_;


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

void ThreadManager::waitAll(int timeOut)
{
    while (timeOut == -1 && joinAllWaiting() > 0)
        sleep(1);
        
}

size_t ThreadManager::joinAllWaiting()
{
    std::vector<THRTYPE_ID>::iterator it;
    for (it = threads_.begin(); it != threads_.end(); ++it)
    {
        if (*it == 0)
            break;

        void* res;
        if (pthread_tryjoin_np(*it, &res) == 0)
        {
            std::vector<THRTYPE_ID>::reverse_iterator rit = threads_.rbegin();
            *it = *rit;
            *rit = 0;
        }
    }

    if (it != threads_.end())
        threads_.erase(it, threads_.end());

    return threads_.size();
}


THRTYPE_ID ThreadManager::_launchThread(THRTYPE_FUNCTION function, void* handler)
{
    THRTYPE_ID thread_id = 0;
#if CONFIG_HAS_PTHREADS
    int st = pthread_create(&thread_id, NULL, 
                            function,
                            handler);

    return (st == 0) ? thread_id : 0;
#else
#ifdef WIN32
#ifdef USE_MFC
    VERIFY(AfxBeginThread((AFX_THREADPROC) function, handler));
#else
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

    return thread_id;
}



