#
#   Copyright (C) 2012
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

CC = g++
#CFLAGS = -Wall -O2 -I.
CCFLAGS = -Wall -g -O2 -I$(TOP) -I$(INCDIR)

OBJS := IPAddress.o Socket.o SocketAddr.o SocketState.o Stream.o ss_ipc.o

SRCS := $(OBJS:.o=.cpp)

INCS = $(IDIR2)/IPAddress.h $(IDIR2)/IPC.h $(IDIR2)/SocketAddr.h $(IDIR2)/Socket.h \
       $(IDIR2)/Stream.h $(IDIR2)/SocketState.h $(IDIR2)/sstypes.h $(TOP)/config.h

LIBSOCKSTR = libsockstr.a


.cpp.o: ; $(CC) $(CCFLAGS) -c $<


all: $(LIBSOCKSTR)

$(LIBSOCKSTR): $(OBJS)
	$(AR) rsv $@ $(OBJS)


.PHONY: clean
clean:
	rm -f $(OBJS) $(LIBSOCKSTR) .makedepends
	touch -r .mkdep-timestamp .makedepends

.PHONY: depends
depends: $(SRCS) $(INCS)
	$(CC) -I$(TOP) -I$(INCDIR) -MM $(SRCS) > .makedepends
	touch .mkdep-timestamp

.makedepends:
include .makedepends
