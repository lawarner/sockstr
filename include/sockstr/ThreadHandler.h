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

#pragma once

#include <signal.h>
#include <vector>
#include <sockstr/sstypes.h>

#ifdef TARGET_LINUX
#include <pthread.h>
#endif

namespace sockstr
{

//
// Macro Definitions
//
#ifndef DllExport
#define DllExport
#endif

// Define a type for the return value of threads since this is system dependent.
#ifdef TARGET_WINDOWS
# define THRTYPE_ID DWORD
# ifdef USE_MFC
#  define THRTYPE UINT
# else
#  define THRTYPE DWORD
#  define THRTYPE_NOCAST
# endif
#else
# define THRTYPE LPVOID
# define WINAPI
# define THRTYPE_ID pthread_t
#endif

//
// FORWARD CLASS DECLARATIONS
//
struct IOPARAMS;
class ThreadManager;

//
// TYPE DEFINITIONS
//
typedef THRTYPE WINAPI THRTYPE_FUNCTION(LPVOID);


//
// CLASS DEFINITIONS
//
/**
 * Interface definition for a threads.  Any thread class should inherit
 * this interface and override the handle() method.
 * The template parameter T specifies the type of data that is passed
 * to the thread handler and the R parameter specifies the return type
 * of the handler routine (default is void).
 */
template <typename T, typename R=void>
class DllExport ThreadHandler
{
public:
    virtual R handle(T data) = 0;

    THRTYPE getStatus() const { return status_; }
    THRTYPE_ID getThreadId() const { return tid_; }
    bool kill(int sig)
    {
        int ret = pthread_kill(tid_, sig);
        return ret == 0;
    }
    void setData(T data) { data_ = data; }
    void setThreadId(THRTYPE_ID tid) { tid_ = tid; }
    void wait() {
        void* res;
        pthread_join(tid_, &res);
    }

protected:
    static THRTYPE WINAPI hookHandle_(LPVOID data)
    {
        ThreadHandler<T,R>* handleThis = (ThreadHandler<T,R>*) data;

        // ignoring any return value for now
        handleThis->handle(handleThis->data_);

        DWORD  dwReturn = 0;
#ifdef THRTYPE_NOCAST
        return dwReturn;
#else
        return reinterpret_cast<THRTYPE>(dwReturn);
#endif
    }

protected:
    T data_;

    THRTYPE status_;  // thread return status

    THRTYPE_ID tid_;  // thread ID

    friend class ThreadManager;
};



/**
 * This class manages the creation and execution of threads.
 * This is a convenience so the rest of the library does not have to worry
 * about threads that are platform specific.
 */
class DllExport ThreadManager
{
public:
    ThreadManager() { }
    virtual ~ThreadManager()
    {
        waitAll();
    }

    template<typename T, typename R>
    static bool create(ThreadHandler<T,R>* handler, bool start = true) {
            THRTYPE_ID thread_id;
            thread_id = _launchThread(handler->hookHandle_, (void*)handler);
            handler->setThreadId(thread_id);
            if (thread_id > 0) {
                threads_.push_back(thread_id);
                return true;
            }
            return false;
        }

    /**
     * Joins any threads that have already exited and removes them from our
     * list.
     *
     * @return Number of threads still running.
     */
    virtual size_t joinAllWaiting();
    virtual void start() { }
    /**
     * Wait for all threads to terminate by doing a join.
     * @param timeOut Number of milliseconds to wait for threads to exit.
     *                If -1, then this function blocks until all theads exit.
     */
    virtual void waitAll(int timeOut = 0);

protected:
    static THRTYPE_ID _launchThread(THRTYPE_FUNCTION function, void* handler);

    static std::vector<THRTYPE_ID> threads_;

private:

};

}  // namespace sockstr
