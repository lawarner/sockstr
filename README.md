sockstr
=======

Socket streams for C++

The sockstr library provides a C++ wrapper around streaming with sockets.

The main purpose of this library is to provide a standard C++ iostream interface
and to make sockets as easy as possible.
A number of I/O modes are supported for Streams such as asynchronous, 
polling, blocking, etc.  These can be used interchangably on an open Stream.

This library is compatible with Windows, MacOS and Linux 32 and 64 bits, Linux ARM and has recently been ported to Android.
For MacOS I have built and tested it using command-line 'make'. I installed libssl from [Homebrew](https://brew.sh).

Note that I have not tested on Windows in a few years.

Sample code
===========
     #include <sockstr/Socket.h>
     
     sockstr::Socket sock("localhost:8080", sockstr::Socket::modeReadWrite);
     if (sock.good())
         sock << "Hello, world!" << std::endl;


How to Build on Linux
=====================

## Dependencies

You will need GNU make, g++ that supports C++17 (or later standard).
If you want TLS/SSL, then you also need openssl library and headers.  Double check 
the config.h file and various Makefile's in this project and adjust accordingly.

Optionally you can generate documentation if you have doxygen installed.  Just 
type 'make doc'.

This library was originally written on Windows with winsock, but has since been ported
to Linux.  During the port, the winsock functionality has been neglected.  I have done
an initial port back to windows, but it needs much more testing.

## Building
Simply type 'make'.  This will build the sockstr library. To build the example programs, type 'make examples'

You can also install the sockstr library by typing 'make install.' You may need to use sudo, depending on where you are installing the files. By default, it installs to /usr/local/lib and /usr/local/include.


How to Build for Android
========================

## Dependencies

You will need Android NDK installed, version 10 or newer, GNU make.
Set the NDK_ROOT environment variable to the directory where the NDK is installed.
Make a standalone toolchain using ${NDK_ROOT}/build/tools/make-standalone-toolchain.sh.
The Makefile assumes ${HOME}/toolchains/android-toolchain so edit as needed.

## Building
Type 'make android'.  This will build the sockstr library but not the example programs.


How to Build on Windows
=======================

## Dependencies

I tried to keep the dependencies to a minimum, so for Windows you only need
Visual Studio 2010 C++ Express.

## Building

In Visual Studio, open the solution in sockstr/vs2010/sockstr.sln.  By default this will build 
the sockstr.lib library and a simple test program in the examples/ directory.


> Note: The test harness tool that was in ipctest subdirectory has been moved to its own project.



# Branches

The code was originally developed in 1996 with (legacy) C++9x. Further development on the 'master' branch will use modern C++, which is C++17 at this time.

The branch 'cpp98' is a snapshot before any modern C++ is used, in case there is some obscure reason it is needed.

# TO DO

- Implement multicast for IPv4 and IPv6
- Use cmake to build and install
- Possibly eliminate the Stream base class since sockets are already derived from std::iostream


# Copyright Information

The sockstr library is provided under the LGPL version 2.1 or later.
