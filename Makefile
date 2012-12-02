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


BACKUPFILE = ~/Downloads/srcrepo/sockstr-src-`date +%Y%j`.tar.bz2

LIBSOCKSTR = libsockstr.a

#SUBDIRS = src examples ipctest
SUBDIRS = src examples


all: subdirs

.PHONY: doc
doc:
	make -C doc

# Backup the whole source dir.
#  Probably want to do "make clean" beforehand, but not required.
backup:
	tar cvaf $(BACKUPFILE) --exclude='.git/*' -C .. sockstr

subdirs:
	for dir in $(SUBDIRS); do \
		make -C $$dir; \
	done


.PHONY: clean
clean:
	for dir in $(SUBDIRS); do \
		make -C $$dir clean; \
	done

.PHONY: depends
depends:
	for dir in $(SUBDIRS); do \
		make -C $$dir depends; \
	done
