## Introduction
Taf(Trading Application Foundations) provides a framework for low-latency trading system development based on Intel x86-64.

## Features
* Based on Intel TBB, SIMD(AVX2+), freelock queue, etc to optimize the performance.
* Nanosecond timestamp available.
* Developed with standard C++, corss-platform (windows and linux) is definitely supported.
* All dependencies included in pack, no need to download any other libs (like TBB).
* Provide C interface for multiple language calling.
* The delay for a simple application could be less 10us.

## Before Compilation
* Check your hardware to make sure that AVX2 or AVX512F is supported.
* C++17 compiler is needed.
* TBB libs and binary located in taf/libs/tbb/bin/

## Compilation
**Windows**
  * Must use Visual Studio 2019 or above
  * open taf/master/build/msvs/taf.sln
  * Must select x64 mode
  * Open taf/master/src/libs/taf/common/simd/simd.hpp to switch USE_AVX512 or USE_AVX512_BELOW micro (already existed in the file), which depends on your hardware.
  * Add /std:c++latest to C/C++ -> Command Line
  * Config TBB env path in Configration->Debugging->Environment: path=..., could refer the properties config in example
  
**Linux**
  * gcc 8.2 or above
  * Example makefile is in taf/master/build/
  * Must add -mavx2（-mavx512f） -std=c++17 to g++ command line
  * Config TBB env path like LD_LIBRARY_PATH=... : /taf/libs/tbb/bin/linux
  * example：  
    $ cd taf/master/build  
    $ make  
    $ ./example
