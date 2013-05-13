// config.h : include file containing standard definitions used in
//            the SockStr classes.
//
/*
 * This file is used internally by the sources in this library.  Do not
 * include in any header files, since this config.h is not distributed.
 */

#ifndef _CONFIG_H_INCLUDED_
#define _CONFIG_H_INCLUDED_

#ifdef linux
#define TARGET_LINUX
#define CONFIG_HAS_OPENSSL  1
#define CONFIG_HAS_PTHREADS 1
#else
#define TARGET_WINDOWS
#endif

#include <sys/types.h>
#include <sockstr/sstypes.h>

#define VERIFY assert
//#include <cerrno>

#if CONFIG_HAS_PTHREADS
#include <pthread.h>
#endif

#endif
