#pragma once

#pragma warning(disable:4244)

#ifdef _MSC_VER
#if _MSC_VER < 1920 // for visual studio 2017
static int __builtin_clz(uint32_t type)
{
	if (_unlikely(0 == type))
		return sizeof(type) << 3;

	int num = 0;
	while (!(type & 0x80000000))
	{
		num += 1;
		type <<= 1;
	}
	return num;
}

static int __builtin_ctzll(uint64_t type)
{
	if (_unlikely(0 == type))
		return sizeof(type) << 3;

	int num = 0;
	while (!(type & 1))
	{
		++num;
		type >>= 1;
	}
	return num;
}

static int __builtin_ctzl(long type)
{
	if (_unlikely(0 == type))
		return sizeof(type) << 3;
	return __builtin_ctzll(type);
}

static __forceinline int __builtin_ctz(uint32_t type)
{
	if (_unlikely(0 == type))
		return sizeof(type) << 3;
	return __builtin_ctzll(type);
}
#endif
#endif

