// config.h : include file containing standard definitions used in
//            the SockStr classes.
//

#ifndef _CONFIG_H_INCLUDED_
#define _CONFIG_H_INCLUDED_

#ifdef linux
#define TARGET_LINUX
#define USE_PTHREADS 1
#else
#define TARGET_WINDOWS
#endif

#include <sys/types.h>
#include <sockstr/sstypes.h>

#define VERIFY assert
//#include <cerrno>

#if USE_PTHREADS
#include <pthread.h>
#endif

#endif
