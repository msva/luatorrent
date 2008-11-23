==================
Overview
==================

luatorrent is a module wrapping the functionality of libtorrent to allow 
developers to write bittorrent clients in Lua. Not all of libtorrent's 
functionality has been exposed (yet), but simple functionality is available.

==================
Prerequisites
==================

* Lua 5.1 + development libraries
* Libtorrent 0.12 + development libraries
* Boost 1.33.1 + development libraries

All prerequisites (plus all development libraries) MUST be installed prior to 
building luatorrent. 

==================
Build Guide
==================

Under Linux run
    make linux

Under OSX run
    make macosx

==================
Windows
==================

Under Windows the build process is more complicated than under *nix. Assuming
you have a copy of Vistual Studio, The steps used to build the library are as 
follows:

1. Grab the prebuilt boost binaries from:

    http://www.boost-consulting.com/products/free

Use version 1.35.0.

2. Grab the latest libtorrent stable source from

    http://www.rasterbar.com/products/libtorrent/index.html

3. Under VS, create a new MFC DLL project. Turn off PCH support, and add all the
CPP and H files from the luatorrent directory. Add the following pre-processor 
defines:

    WIN32
    _WINDOWS;
    _USRDLL
    WIN32_LEAN_AND_MEAN
    _WIN32_WINNT=0x0500
    BOOST_ALL_NO_LIB
    _FILE_OFFSET_BITS=64
    BOOST_THREAD_USE_LIB
    UNICODE
    TORRENT_USE_OPENSSL
    NDEBUG

I also imported the source to libtorrent into the luatorrent project, as I had 
problems using libtorrent as a DLL.

Add the following Additional Dependancies:
    libboost_system-vc$VER-mt-1_35.lib
    libboost_filesystem-vc$VER-mt-1_35.lib
    libboost_date_time-vc$VER-mt-1_35.lib
    libboost_thread-vc$VER-mt-1_35.lib
    lua51.lib
    libeay32.lib
    ssleay32.lib

Add the following Additional Include Header Files:
    <path to>\boost-1_35_0\
    <path to>\libtorrent\include\
    <path to>\libtorrent\include\libtorrent\
    <path to>\libtorrent\zlib\
    <path to>\OpenSSL\include\
    

The DLL should build without errors.

I STRONGLY suggest you use the luatorrent binary package for Windows I've 
provided. 

==================
Examples
==================

dump_torrent.lua:
    Load a .torrent file and extract some of the information available.

simple_client.lua:
    Create a session object, add a torrent file to the session and download the
    torrent's files.

==================
TODO
==================

 * Finish wrapping all of libtorrents functionality
 * Add more error checking when calling C++ code
 * Provide support for Cygwin and/or Mingw32
