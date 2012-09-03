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

CC = g++
#CFLAGS = -Wall -O2 -I..
CCFLAGS = -Wall -g -O2 -DTARGET_LINUX=1 -I$(TOP) -I$(INCDIR)
LDFLAGS = -pthread $(LIBSOCKLIB)

#COMPILE = $(CC) $(CFLAGS) -c

OBJS := simplest.o filecopy.o fbread.o fb2read.o testsockstr.o

SRCS := $(OBJS:.o=.cpp)

INCS = 

LIBSOCKLIB = $(TOP)/src/libsockstr.a

PROGRAMS = simplest filecopy fbread fb2read testsockstr


.cpp.o: ; $(CC) $(CCFLAGS) -c $<


all: $(PROGRAMS)

simplest: simplest.o $(LIBSOCKLIB)

filecopy: filecopy.o $(LIBSOCKLIB)

fbread: fbread.o $(LIBSOCKLIB)

fb2read: fb2read.o $(LIBSOCKLIB)

testsockstr: testsockstr.o $(LIBSOCKLIB)



.PHONY: clean
clean:
	rm -f $(OBJS) $(PROGRAMS)

.PHONY: depends
depends: $(SRCS) $(INCS)
	$(CC) -I. -MM $(SRCS) > .makedepends
	touch .mkdep-timestamp

.makedepends:
include .makedepends
