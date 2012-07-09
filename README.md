sockstr
=======

Socket streams for C++

The sockstr library provides a C++ wrapper around streaming with sockets.
A number of I/O modes are supported and a Stream such as asynchronous, 
polling, etc.  These can be changed at any time for an open Stream.

How to Build on Linux
=====================

## Dependencies

You will need GNU make, g++ and pthreads to build this package.  Optionally you can 
generate documentation if you have doxygen.

This library was originally written on Windows with winsock, but has since been ported
to Linux.  During the port, the winsock functionality has broken.  I plan to fix this
in a future version.

## Building
Simply type 'make'.

# Copyright Information

The sockstr library is provided with the LGPL.
