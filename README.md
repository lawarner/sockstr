sockstr
=======

Socket streams for C++

The sockstr library provides a C++ wrapper around streaming with sockets.
The main purpose of this library is to interface with iostream from the 
standard C++ library.
A number of I/O modes are supported and a Stream such as asynchronous, 
polling, etc.  These can be used interchangably on an open Stream.

This library is compatible with Windows and Linux, and has recently been ported to ARM
Linux using the Raspberry Pi toolchain.

Sample code
===========
     #include <sockstr/Socket.h>
     
     sockstr::Socket sock("localhost:8080", sockstr::Socket::modeReadWrite);
     if (sock.good())
         sock << "Hello, world!" << std::endl;


Note: The test harness tool that was in ipctest subdirectory has been moved to its
      own project.


How to Build on Linux
=====================

## Dependencies

You will need GNU make, g++ and pthreads to build this package.
If you want TLS/SSL, then you also need openssl library and headers.  Double check 
the config.h file and various Makefile's in this project and adjust accordingly.

Optionally you can generate documentation if you have doxygen installed.  Just 
type 'make doc'.

This library was originally written on Windows with winsock, but has since been ported
to Linux.  During the port, the winsock functionality has been neglected.  I have done
an initial port back to windows, but it needs much more testing.

The primary purpose of this library is to provide a standard C++ iostream interface,
and to make sockets as easy as possible.

## Building
Simply type 'make'.  This will build the sockstr library and example programs.


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
