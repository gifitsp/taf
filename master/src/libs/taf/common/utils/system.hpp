#pragma once

#pragma warning(disable:4996)

namespace taf::common::system
{
	static inline void* memcpy(void* destin, void* source, size_t size)
	{
		return ::memcpy(destin, source, size);
	}
}