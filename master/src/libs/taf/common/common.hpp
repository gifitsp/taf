#pragma once

#pragma warning(disable:4103)

#include "defines.h"
#include "stdheaders.h"

#ifdef USE_SIMD
#include "simd/simd.hpp"
#else

static _force_inline size_t _strfind(const char* str, const char* substr)
{
	auto ret = std::strstr(str, substr);
	if (ret != nullptr)
		return ret - str;
	return -1;
}

static _force_inline size_t _strfind(const std::string& str, const std::string& substr)
{
	return _strfind(str.c_str(), substr.c_str());
}

#define fast_strfind _strfind
#endif

#include "utils/system.hpp"
#include "utils/convert.hpp"
#include "utils/string.hpp"
#include "utils/time.hpp"
#include "utils/lock_free.hpp"
#include "utils/sync.hpp"
#include "utils/buffer.hpp"