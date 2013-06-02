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

#ifndef _THREADHANDLER_H_INCLUDED_
#define _THREADHANDLER_H_INCLUDED_
//
//

//
// INCLUDE FILES
//
#include <sockstr/sstypes.h>


namespace sockstr
{

//
// MACRO DEFINITIONS
//
#ifndef DllExport
#define DllExport
#endif

// Define a type for the return value of threads since this is system dependent.
#ifdef TARGET_WINDOWS
# ifdef USE_MFC
#  define THRTYPE UINT
# else
#  define THRTYPE DWORD
#  define THRTYPE_NOCAST
# endif
#else
# define THRTYPE LPVOID
# define WINAPI
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
    void setData(T data) { data_ = data; }

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

#ifdef _DEBUG
	static void* m_pLastBuffer;	// Last buffer used for overlapped I/O
#endif
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
    virtual ~ThreadManager() { }

    /** This is for backwards compatibility only and will be removed soon. */
    static bool
        create(THRTYPE_FUNCTION function, void* data, bool start = true);

    template<typename T, typename R>
    static bool
        create(ThreadHandler<T,R>* handler, bool start = true)
        {
            return _launchThread(handler->hookHandle_, (void*)handler);
        }

    virtual void start() { }

protected:
    static bool _launchThread(THRTYPE_FUNCTION function, void* handler);

private:

};

}  // namespace sockstr

#endif // _THREADHANDLER_H_INCLUDED_
