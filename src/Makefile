#
#   Copyright (C) 2012 - 2015
#   Andy Warner
#   This file is part of the sockstr class library.
#
#   The sockstr class library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License as published by the Free Software Foundation; either
#   version 2.1 of the License, or (at your option) any later version.
#
#   The sockstr class library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with the sockstr library; if not, write to the Free
#   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
#   02111-1307 USA.  */

# Simple makefile for the sockstr class library

TOP = ..
INCDIR = $(TOP)/include
IDIR2 = $(INCDIR)/sockstr

#AR = ar
CC = g++
#CCFLAGS = -Wall -g -O2 -I$(TOP) -I$(INCDIR)
CCFLAGS = -std=c++17 -Wall -g -O0 -I$(TOP) -I$(INCDIR)

OBJS := Socket.o SocketAddr.o StreamBuf.o SocketState.o SocketStateTLS.o \
        Stream.o HttpHelpers.o HttpStream.o OAuth.o

SRCS := $(OBJS:.o=.cpp)

INCS = $(IDIR2)/IPC.h $(IDIR2)/SocketAddr.h $(IDIR2)/StreamBuf.h \
       $(IDIR2)/Socket.h $(IDIR2)/Stream.h $(IDIR2)/SocketState.h $(IDIR2)/HttpHelpers.h \
       $(IDIR2)/HttpStream.h $(IDIR2)/OAuth.h \
       $(IDIR2)/sstypes.h $(TOP)/config.h

LIBSOCKSTR = libsockstr.a

LIBOPENSSL = -lssl -lcrypto


.cpp.o: ; $(CC) $(CCFLAGS) -c $<


all: $(LIBSOCKSTR)

$(LIBSOCKSTR): $(OBJS)
	$(AR) rsv $@ $(OBJS)

$(IDIR2)/sstypes.h: $(IDIR2)/sstypes-linux.h $(IDIR2)/sstypes-armlinux.h $(IDIR2)/sstypes-armandroid.h
	cd $(IDIR2) ; \
	echo CC is ${CC} ; \
	case ${CC} in \
	    *androideabi-g++) ln -sf sstypes-armandroid.h sstypes.h ;; \
	    *gnueeabif-g++) ln -sf sstypes-armlinux.h sstypes.h ;; \
	    *)  ln -sf sstypes-linux.h sstypes.h ;; \
	esac

.PHONY: clean
clean:
	rm -f $(OBJS) $(LIBSOCKSTR)

.PHONY: depends
depends: $(SRCS) $(INCS)
	$(CC) -I$(TOP) -I$(INCDIR) -MM $(SRCS) > .makedepends
	touch .mkdep-timestamp

.makedepends:
include .makedepends
