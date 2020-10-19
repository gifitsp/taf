#pragma once

/*
Note: compiler must support AVX2 or above

refer
https://stackoverflow.com/questions/28939652/how-to-detect-sse-sse2-avx-avx2-avx-512-avx-128-fma-kcvi-availability-at-compile
for more details
*/

#ifdef _MSC_VER

extern "C" int __isa_available;

static
struct _pre_detector_
{
	_pre_detector_()
	{
		if (__isa_available < 5)
		{
			throw std::invalid_argument("AVX2 not supported on this machine");
		}
	}
} __pre_detector__;

//#define USE_AVX512
#define USE_AVX512_BELOW

#include <intrin.h>

#else // gcc

///cmd:
///gcc -march=knl -dM -E - < /dev/null | egrep "SSE|AVX" | sort

#include <x86intrin.h>

#ifdef __AVX512F__
#define USE_AVX512
#elif defined(__AVX2__)
#define USE_AVX512_BELOW
#endif

#endif

#include "funcs.h"


#ifdef USE_AVX512
#pragma message("AVX512 available")
#include "details/avx512f-strstr-v2.h"
#define fast_strfind avx512f_strstr_v2

#elif defined(USE_AVX512_BELOW)
#pragma message("AVX2 available")
#include "details/avx2-naive-strstr64.h"
#define fast_strfind avx2_naive_strstr64

#else
#error no simd type(AVX2 or AVX512F) support, please check your CPU
#endif