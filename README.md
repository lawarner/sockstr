sockstr
=======

Socket streams for C++

The sockstr library provides a C++ wrapper around streaming with sockets.
The main purpose of this library is to interface with iostream from the 
standard C++ library.
A number of I/O modes are supported and a Stream such as asynchronous, 
polling, etc.  These can be used interchangably on an open Stream.

Sample code
===========
     #include <sockstr/Socket.h>
     
     sockstr::Socket sock("localhost:8080", sockstr::Socket::modeReadWrite);
     if (sock.good())
         sock << "Hello, world!" << std::endl;

There is also a test harness in the ipctest subdirectory.  This allows capturing test 
scenarios and saving these for later playback.


How to Build on Linux
=====================

## Dependencies

You will need GNU make, g++ and pthreads to build this package.  Optionally you can 
generate documentation if you have doxygen.

This library was originally written on Windows with winsock, but has since been ported
to Linux.  During the port, the winsock functionality has broken.  I plan to fix this
in a future version.

## Building
Simply type 'make'.  This will build the sockstr library and example programs.

To build the ipctest library and tools, type 'make ipctest'.  Additional prerequites 
are required to build ipctest, such as:

- expat
- gtkmm


How to Build on Windows
=======================

## Dependencies

I tried to keep the dependencies to a minimum, so for Windows you only need
Visual Studio 2010 C++ Express.

## Building

In Visual Studio, open the solution in sockstr/vs2010/sockstr.sln.  By default this will build 
the sockstr.lib library and a simple test program in the examples/ directory.

# Copyright Information

The sockstr library is provided under the LGPL version 2.1 or later.
