// config.h : include file containing standard definitions used in
//            the SockStr classes.
//

#ifndef _CONFIG_H_INCLUDED_
#define _CONFIG_H_INCLUDED_

#ifdef linux
#define USE_PTHREADS 1
#endif

#include <sys/types.h>
#include <sockstr/sstypes.h>

#define VERIFY assert

#if USE_PTHREADS
#include <pthread.h>
#endif

#endif
