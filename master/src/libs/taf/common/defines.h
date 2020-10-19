#pragma once

#include <stdint.h>

#if defined(_M_X64) || defined(_M_IA64) || defined(__x86_64__) || defined(__x86_64)
#else
#error x64 compiler needed
#endif

#if defined(_WINDOWS) || defined(WIN32) || defined(_MSC_VER)
#define WINDOWS
#endif

#ifdef _MSC_VER

#if _MSC_VER < 1923
#error visual studio 2019(16.3) or above required
#endif

#ifndef _likely
#define _likely(x) x
#endif
#ifndef _unlikely
#define _unlikely(x) x
#endif

#define _force_inline __forceinline
#define _never_inline __declspec(noinline)

#define _export       __declspec( dllexport )
#define _import       __declspec( dllimport )

#define EXIT_PROCESS(code) exit(code)

#else

#if __cplusplus < 201703L
#error compiler must support C++17
#endif

#ifndef _likely
#define _likely(x) __builtin_expect(!!(x), 1)
#endif
#ifndef _unlikely
#define _unlikely(x) __builtin_expect(!!(x), 0)
#endif

#define _force_inline inline __attribute__((always_inline, unused))
#define _never_inline inline __attribute__((noinline, unused))

#define _export
#define _import

#define EXIT_PROCESS(code) (kill(getpid(), SIGKILL))

#endif

typedef int16_t             int16;
typedef uint16_t            uint16;
typedef int32_t             int32;
typedef uint32_t            uint32;
typedef uint32_t            uint;
typedef int64_t             int64;
typedef uint64_t            uint64;

#define STR(x) #x 

#define DEFAULT_SIZE 64
#define DOUBLE_EPSLION 0.00000001

#define USE_SIMD
#define USE_MEMPOOL

#pragma pack(push)
#pragma pack(8)
