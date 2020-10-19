#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

namespace taf::common::utils
{
	/// <summary>
	/// char bytes to type T
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="str"></param>
	/// <returns></returns>
	template<typename T>
#ifdef WINDOWS
	static
#endif
	inline T to_type(const char* str)
	{
		if (nullptr == str || std::strcmp(str, "") == 0)
			return 0;

		T value;
		std::istringstream is(str);
		is >> value;
		return value;
	}

	/// <summary>
	/// string to type T
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="str"></param>
	/// <returns></returns>
	template<typename T>
#ifdef WINDOWS
	static
#endif
	inline T to_type(const std::string& str)
	{
		return to_type<T>(str.c_str());
	}

	/// <summary>
	/// type T to string
	/// num could be ignore unless T is double
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="value"></param>
	/// <param name="num"></param>
	/// <returns></returns>
	template<typename T>
#ifdef WINDOWS
	static
#endif
	inline std::string to_string(T value, int num = 0)
	{
		return std::to_string(value);
	}

	/// <summary>
	/// num indicates the decimal number as string
	/// </summary>
	/// <param name="value"></param>
	/// <param name="num"></param>
	/// <returns></returns>
	template<>
#ifdef WINDOWS
	static
#endif
	inline std::string to_string<double>(double value, int num)
	{
		char buf[64];
		std::snprintf(buf, 64, "%.*f", num, value);
		return buf;
	}
}