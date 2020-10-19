#pragma once

#ifdef WINDOWS
#define FORCE_INLINE __forceinline
#define MAYBE_UNUSED [[maybe_unused]]

#ifndef _likely
#define _likely(x) x
#endif
#ifndef _unlikely
#define _unlikely(x) x
#endif

#else
#define FORCE_INLINE inline __attribute__((always_inline))
#define MAYBE_UNUSED inline __attribute__((unused))

#ifndef _likely
#define _likely(x) __builtin_expect(!!(x), 1)
#endif
#ifndef _unlikely
#define _unlikely(x) __builtin_expect(!!(x), 0)
#endif

#endif

#if defined(HAVE_NEON_INSTRUCTIONS)
#   include <arm_neon.h>
#   define USE_SIMPLE_MEMCMP // for fixed-memcmp.cpp
#else
#   include <immintrin.h>
#endif

namespace bits {

	template <typename T>
	T clear_leftmost_set(const T value) {

		assert(value != 0);

		return value & (value - 1);
	}


	template <typename T>
	unsigned get_first_bit_set(const T value) {

		assert(value != 0);

		return __builtin_ctz(value);
	}


	template <>
	unsigned get_first_bit_set<uint64_t>(const uint64_t value) {

		assert(value != 0);

		return __builtin_ctzl(value);
	}

} // namespace bits

namespace avx512 {

	union proxy {
		__m512i  vec;
		uint8_t  u8[64];
		uint16_t u16[32];
	};


	namespace dump {

		void epu16(const __m512i vec) {

			proxy p;
			p.vec = vec;

			for (int i = 0; i < 32; i++) {
				printf("%04x ", p.u16[i]);
			}

			putchar('\n');
		}

		void epu8(const __m512i vec) {

			proxy p;
			p.vec = vec;

			putchar('\'');
			for (int i = 0; i < 64; i++) {
				printf("%02x ", p.u8[i]);
			}

			putchar('\'');
			putchar('\n');
		}

	} // namespace dump

} // namespace sse

size_t avx2_strlen(const char *s)
{
	unsigned int i = 0;
	const __m256i *p;
	__m256i mask, zero;

	p = (__m256i *) s;
	zero = _mm256_setzero_si256();

	while (1) {
		mask = _mm256_cmpeq_epi8(*p, zero);
		if (!_mm256_testz_si256(mask, mask)) {
			return (i * 32) + std::strlen((char *)p);
		}
		i++;
		p++;
	}
}

#include "fixed-memcmp.h"