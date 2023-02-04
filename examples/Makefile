#
#   Copyright (C) 2012, 2013
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

CC = g++
CCFLAGS = -std=c++17 -Wall -g -O0 -DTARGET_LINUX=1 -I$(TOP) -I$(INCDIR)
#CCFLAGS = -Wall -g -DTARGET_LINUX=1 -I$(TOP) -I$(INCDIR)
DEPCPPFLAGS = -std=c++17 -Wall -g -O2 -DTARGET_LINUX=1 -I$(TOP) -I$(INCDIR)
LDFLAGS = -pthread
LDLIBS = 

OBJS :=  asyncsock.o echoserver.o fbread.o fb2read.o filecopy.o httptest.o \
         readsdp.o restclient.o restserver.o simplest.o testsockstr.o
SRCS := $(OBJS:.o=.cpp)

INCS = 

DEPLIBS = $(LIBSOCKLIB) $(LIBOPENSSL)

LIBSOCKLIB = $(TOP)/src/libsockstr.a
LIBOPENSSL = -lssl -lcrypto

PROGRAMS = asyncsock echoserver fbread fb2read  filecopy httptest \
           readsdp restclient restserver simplest testsockstr


.cpp.o: ; $(CC) $(CCFLAGS) -c $<


all: $(PROGRAMS)

$(PROGRAMS): $(DEPLIBS)

.PHONY: clean
clean:
	rm -f $(OBJS) $(PROGRAMS)

.PHONY: depends
depends: $(SRCS) $(INCS)
	$(CC) -I. $(DEPCPPFLAGS) -MM $(SRCS) > .makedepends
	touch .mkdep-timestamp

.makedepends:
include .makedepends
