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

TOP = ../..
INCDIR = $(TOP)/include

SUBDIRS = 

CC = g++
#CFLAGS = -Wall -O2 -I..
CPPFLAGS = -Wall -g -O2 -DTARGET_LINUX=1 -I$(TOP) -I$(INCDIR)
CCFLAGS = -Wall -g -O2 -DTARGET_LINUX=1 -I$(TOP) -I$(INCDIR)
LDFLAGS = -pthread $(LIBSOCKLIB)

#COMPILE = $(CC) $(CFLAGS) -c

OBJS := BuiltinCommands.o Field.o FieldType.o Message.o \
	Params.o Parser.o RunContext.o Serializer.o TestBase.o

SRCS := $(OBJS:.o=.cpp)

INCS = $(echo *.h)

LIBIPCTEST = libipctest.a

LIBSOCKLIB = $(TOP)/src/libsockstr.a

PROGRAMS = 


# .cpp.o: ; $(CC) $(CCFLAGS) -c $<


all: $(LIBIPCTEST)

$(LIBIPCTEST): $(OBJS) $(LIBSOCKLIB)
	$(AR) rsv $@ $(OBJS)


.PHONY: subdirs
subdirs:
	for dir in $(SUBDIRS); do \
		make -C $$dir; \
	done


.PHONY: clean
clean:
	rm -f $(OBJS) $(LIBIPCTEST)
	for dir in $(SUBDIRS); do \
		make -C $$dir clean; \
	done

.PHONY: depends
depends: $(SRCS) $(INCS)
	$(CC) -I. -MM $(SRCS) > .makedepends
	touch .mkdep-timestamp

.makedepends:
include .makedepends
