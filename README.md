sockstr
=======

Socket streams for C++

The sockstr library provides a C++ wrapper around streaming with sockets.
The main purpose of this library is to interface with iostream from the 
standard C++ library.
A number of I/O modes are supported and a Stream such as asynchronous, 
polling, etc.  These can be used interchangably on an open Stream.

This library is compatible with Windows and Linux 32 and 64 bits, Linux ARM and has recently been ported to Android.

Sample code
===========
     #include <sockstr/Socket.h>
     
     sockstr::Socket sock("localhost:8080", sockstr::Socket::modeReadWrite);
     if (sock.good())
         sock << "Hello, world!" << std::endl;


Note: The test harness tool that was in ipctest subdirectory has been moved to its
      own project.


How to Build on Android
=======================

## Dependencies

You will need Android NDK installed, version 10 or newer, GNU make.
Set the NDK_ROOT environment variable to the directory where the NDK is installed.
Make a standalone toolchain using ${NDK_ROOT}/build/tools/make-standalone-toolchain.sh.
The Makefile assumes ${HOME}/toolchains/android-toolchain so edit as needed.

## Building
Type 'make android'.  This will build the sockstr library but not the example programs.

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
