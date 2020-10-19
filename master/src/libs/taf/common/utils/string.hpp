#pragma once

#include <string>
#include <atomic>
#include <unordered_map>

namespace taf::common
{
	/// <summary>
	/// using simd to speed up if USE_SIMD is defined
	/// </summary>
	/// <typeparam name="Alloc"></typeparam>
	template<class Alloc>
	class fstdstring : public std::basic_string<char, std::char_traits<char>, Alloc>
	{
	public:
		typedef std::basic_string<char, std::char_traits<char>, Alloc> Base;

		fstdstring()
		{
		}

		fstdstring(const char* str)
			: Base(str)
		{
		}

		fstdstring(const std::string& str)
			: Base(str)
		{
		}

		fstdstring(const fstdstring& str)
			: Base(str)
		{
		}

		fstdstring(std::string&& str)
			: Base(str)
		{
		}

		fstdstring& operator=(std::string&& str)
		{
			*(std::string*)this = std::move(str);
			return *this;
		}

		fstdstring& operator=(const std::string& str)
		{
			*(std::string*)this = str;
			return *this;
		}

		fstdstring& operator=(const char* str)
		{
			*(std::string*)this = str;
			return *this;
		}

		inline size_t find(const char* str, size_t offset = 0) const
		{
			if (this->empty() || nullptr == str)
				return std::string::npos;

#ifdef USE_SIMD
			auto pos = fast_strfind(this->data() + offset, this->size() - offset, str, strlen(str));
			if (pos != std::string::npos)
				return offset + pos;
			return pos;
#else
			return ((std::string*)this)->find(str, offset);
#endif
		}

		inline size_t find(const std::string& str, size_t offset = 0) const
		{
			return find(str.c_str(), offset);
		}
	};

	/// <summary>
	/// fast fixed memcpy template
	/// </summary>
	/// <param name="dest"></param>
	/// <param name="src"></param>
	/// <param name="length"></param>
	/// <returns></returns>
	template<uint32_t Size>
#ifdef WINDOWS
	static
#endif
	inline void* fmemcpy(void* dest, const void* src, uint32_t length)
	{
		if (length > 0)
			return std::memcpy(dest, src, length);
		return dest;
		//throw std::runtime_error("[memcpy]wrong instantiation function called");
	}

	template<>
#ifdef WINDOWS
	static
#endif
	inline void* fmemcpy<1>(void* dest, const void* src, uint32_t length)
	{
		*(int8_t*)dest = *(int8_t*)src;
		return dest;
	}

	template<>
#ifdef WINDOWS
	static
#endif
	inline void* fmemcpy<2>(void* dest, const void* src, uint32_t length)
	{
		*(int16_t*)dest = *(int16_t*)src;
		return dest;
	}

	template<>
#ifdef WINDOWS
	static
#endif
	inline void* fmemcpy<4>(void* dest, const void* src, uint32_t length)
	{
		*(int32_t*)dest = *(int32_t*)src;
		return dest;
	}

	template<>
#ifdef WINDOWS
	static
#endif
	inline void* fmemcpy<8>(void* dest, const void* src, uint32_t length)
	{
		*(int64_t*)dest = *(int64_t*)src;
		return dest;
	}

	template<>
#ifdef WINDOWS
	static
#endif
	 inline void* fmemcpy<16>(void* dest, const void* src, uint32_t length)
	{
		*(int64_t*)dest = *(int64_t*)src;
		*(int64_t*)((char*)dest + 8) = *(int64_t*)((char*)src + 8);
		return dest;
	}

	template<>
#ifdef WINDOWS
	static
#endif
	inline void* fmemcpy<32>(void* dest, const void* src, uint32_t length)
	{
		*(int64_t*)dest = *(int64_t*)src;
		*(int64_t*)((char*)dest + 8) = *(int64_t*)((char*)src + 8);
		*(int64_t*)((char*)dest + 16) = *(int64_t*)((char*)src + 16);
		*(int64_t*)((char*)dest + 24) = *(int64_t*)((char*)src + 24);
		return dest;
	}

	template<>
#ifdef WINDOWS
	static
#endif
	inline void* fmemcpy<64>(void* dest, const void* src, uint32_t length)
	{
		fmemcpy<32>(dest, src, 32);
		fmemcpy<32>((char*)dest + 32, (char*)src + 32, length - 32);
		return dest;
	}

	template<>
#ifdef WINDOWS
	static
#endif
	inline void* fmemcpy<128>(void* dest, const void* src, uint32_t length)
	{
		fmemcpy<64>(dest, src, 64);
		fmemcpy<64>((char*)dest + 64, (char*)src + 64, length - 64);
		return dest;
	}

	template<>
#ifdef WINDOWS
	static
#endif
	inline void* fmemcpy<256>(void* dest, const void* src, uint32_t length)
	{
		fmemcpy<128>(dest, src, 128);
		fmemcpy<128>((char*)dest + 128, (char*)src + 128, length - 128);
		return dest;
	}

	typedef std::function<void* (void*, const void*, uint32_t)> MEMCPY_FUNCTION;
	static MEMCPY_FUNCTION memcpy_list[257] = {nullptr};

	static
	class FunctionInitializer
	{
	public:
		FunctionInitializer()
		{
			memcpy_list[1] = fmemcpy<1>;
			memcpy_list[2] = fmemcpy<2>;
			memcpy_list[4] = fmemcpy<4>;
			memcpy_list[8] = fmemcpy<8>;
			memcpy_list[16] = fmemcpy<16>;
			memcpy_list[32] = fmemcpy<32>;
			memcpy_list[64] = fmemcpy<64>;
			memcpy_list[128] = fmemcpy<128>;
			memcpy_list[256] = fmemcpy<256>;
		}
	}_function_initializer;

	static inline void* fmemcpy(void* dest, const void* src, uint32_t length)
	{
		return std::memcpy(dest, src, length);

		//MEMCPY_FUNCTION memcpy_function = memcpy_list[length];
		//if (memcpy_function)
		//{
		//	return memcpy_function(dest, src, length);
		//}
		//else
		//	return std::memcpy(dest, src, length);

		//switch(length)
		//{
		//case 64:
		//	return fmemcpy<64>(dest, src, length);
		//case 32:
		//	return fmemcpy<32>(dest, src, length);
		//default:
		//	return std::memcpy(dest, src, length);
		//}
	}

	/// <summary>
	/// Fast string template implementation. Size should be 32 or 64
	/// Warning: multi-threading unsafe
	/// </summary>
	template<size_t Size>
	class ftstring
	{
		char _buf[Size];
		size_t _size = 0;

		inline void _set(const char* str, size_t size)
		{
			if (size >= Size)
			{
				char error[128];
				std::snprintf(error, 128, "[ftstring::_set]size[%llu] exceeds the max available size[%llu], str: %s", size, Size, str);
				throw std::runtime_error(error);
			}

			if (size > 0)
			{
				_size = size;
				fmemcpy(_buf, str, (uint32_t)_size);
				_buf[_size] = '\0';
			}
			else
			{
				clear();
			}
		}

		inline void _set(const char* str)
		{
			size_t size = 0;
			if (str)
				size = std::strlen(str);
			_set(str, size);
		}

	public:
		typedef ftstring Base;

		class iterator : public std::iterator<std::random_access_iterator_tag, char>
		{
		protected:
			char* _ptr = nullptr;
			char* _org_ptr = nullptr;
			size_t _size = 0;

			friend class ftstring;

		public:
			iterator() {}

			iterator(const char* p, size_t size)
			{
				_ptr = (char*)p;
				_org_ptr = _ptr;
				_size = size;
			}

			iterator(const iterator& iter)
			{
				*this = iter;
			}

			iterator& operator = (const iterator& iter)
			{
				if (this != &iter)
				{
					_ptr = iter._ptr;
					_org_ptr = iter._org_ptr;
					_size = iter._size;
				}
				return *this;
			}

			bool operator != (const iterator& iter)
			{
				return _ptr != iter._ptr;
			}

			bool operator == (const iterator& iter)
			{
				return _ptr == iter._ptr;
			}

			char& operator * ()
			{
				return *_ptr;
			}

			iterator& operator ++ ()
			{
				++_ptr;
				return *this;
			}

			iterator operator ++ (int)
			{
				iterator tmp = *this;
				++_ptr;
				return tmp;
			}

			inline void _decrement()
			{
				_ptr > _org_ptr ? --_ptr : (_ptr = _org_ptr + _size);
			}

			iterator& operator -- ()
			{
				_decrement();
				return *this;
			}

			iterator operator -- (int)
			{
				iterator tmp = *this;
				_decrement();
				return tmp;
			}

			iterator& operator += (uint32_t n)
			{
				_ptr += n;
				return *this;
			}

			iterator& operator -= (uint32_t n)
			{
				_ptr -= n;
				return *this;
			}

			iterator operator + (uint32_t n)
			{
				iterator it = *this;
				it._ptr += n;
				return it;
			}

			iterator operator - (uint32_t n)
			{
				iterator it = *this;
				it._ptr -= n;
				return it;
			}
		};

		class reverse_iterator : public iterator
		{
			friend class ftstring;

		public:
			reverse_iterator() {}

			reverse_iterator(const char* p, size_t size)
				: iterator(p, size)
			{
				iterator::_org_ptr = iterator::_ptr - size + 1;
			}

			reverse_iterator& operator -- ()
			{
				++iterator::_ptr;
				return *this;
			}

			reverse_iterator operator -- (int)
			{
				reverse_iterator tmp = *this;
				++iterator::_ptr;
				return tmp;
			}

			inline void _decrement()
			{
				iterator::_ptr > iterator::_org_ptr ? --iterator::_ptr : (iterator::_ptr = iterator::_org_ptr + iterator::_size);
			}

			reverse_iterator& operator ++ ()
			{
				_decrement();
				return *this;
			}

			reverse_iterator operator ++ (int)
			{
				reverse_iterator tmp = *this;
				_decrement();
				return tmp;
			}

			reverse_iterator& operator -= (uint32_t n)
			{
				iterator::_ptr += n;
				return *this;
			}

			reverse_iterator& operator += (uint32_t n)
			{
				iterator::_ptr -= n;
				return *this;
			}

			reverse_iterator operator - (uint32_t n)
			{
				reverse_iterator it = *this;
				it.iterator::_ptr += n;
				return it;
			}

			reverse_iterator operator + (uint32_t n)
			{
				reverse_iterator it = *this;
				it.iterator::_ptr -= n;
				return it;
			}
		};

	public:
		ftstring(const char* str = "")
		{
			_set(str);
		}

		ftstring(const std::string& str)
		{
			_set(str.c_str(), str.size());
		}

		ftstring(const ftstring& str)
		{
			_set(str.c_str(), str.size());
		}

		ftstring& operator=(const char* str)
		{
			_set(str);
			return *this;
		}

		ftstring& operator=(const std::string& str)
		{
			_set(str.c_str(), str.size());
			return *this;
		}

		ftstring& operator=(const ftstring& str)
		{
			_set(str._buf, str._size);
			return *this;
		}

		ftstring operator+(const char* str) const
		{
			ftstring s = *this;
			s.append(str);
			return s;
		}

		ftstring operator+(const ftstring& str) const
		{
			ftstring s = *this;
			s.append(str);
			return s;
		}

		ftstring operator+(const std::string& str) const
		{
			ftstring s = *this;
			s.append(str);
			return s;
		}

		ftstring& operator+=(const char* str)
		{
			this->append(str);
			return *this;
		}

		ftstring& operator+=(const ftstring& str)
		{
			this->append(str);
			return *this;
		}

		ftstring& operator+=(const std::string& str)
		{
			this->append(str);
			return *this;
		}

		bool operator==(const ftstring& str) const
		{
			return (char*)this->_buf == (char*)str._buf || (this->_size == str._size && 0 == std::memcmp(this->_buf, str._buf, this->_size));
		}

		bool operator==(const std::string& str) const
		{
			return 0 == str.compare(this->_buf);
		}

		bool operator==(const char* str) const
		{
			return 0 == std::strcmp(this->_buf, str);
		}

		bool operator!=(const ftstring& str) const
		{
			return (char*)this->_buf != (char*)str._buf && (this->_size != str._size || 0 != std::memcmp(this->_buf, str._buf, this->_size));
		}

		bool operator!=(const std::string& str) const
		{
			return 0 != str.compare(this->_buf);
		}

		bool operator!=(const char* str) const
		{
			return 0 != std::strcmp(this->_buf, str);
		}

		friend std::ostream& operator << (std::ostream& os, const ftstring& str)
		{
			os << str.c_str();
			return os;
		}

		inline bool empty() const
		{
			return 0 == _size;
		}

		inline size_t capacity() const
		{
			return Size;
		}

		inline size_t size() const
		{
			return _size;
		}

		inline size_t length() const
		{
			return _size;
		}

		inline const char* c_str() const
		{
			return _buf;
		}

		inline const char* data() const
		{
			return _buf;
		}

		inline iterator begin() const
		{
			return iterator(_buf, _size);
		}

		inline iterator end() const
		{
			return iterator(_buf + _size, _size);
		}

		inline reverse_iterator rbegin() const
		{
			if (0 == _size)
				return rend();
			return reverse_iterator(_buf + _size - 1, _size);
		}

		inline reverse_iterator rend() const
		{
			return (reverse_iterator&)end();
		}

		inline ftstring& replace(size_t offset, int count, const char* str)
		{
			if (!this->empty() && offset < _size && count > 0 && str != nullptr)
			{
				count = offset + count > _size ? _size - offset : count;

				auto oldlen = count;
				auto newlen = std::strlen(str);
				auto size = _size - oldlen + newlen;
				if (size >= Size)
				{
					char error[128];
					std::snprintf(error, 128, "[ftstring::replace]no enough buffer size to replace, %llu >= %llu, str: %s", size, Size, _buf);
					throw std::runtime_error(error);
				}

				auto begin = _buf + offset;
				auto end = begin + count;
				auto restlen = _size - offset - count;
				if (oldlen >= newlen)
				{
					fmemcpy(begin, str, newlen);
					if (oldlen > newlen)
					{
						std::memmove(begin + newlen, end, restlen);
					}
				}
				else
				{
					std::memmove(end + newlen - oldlen, end, restlen);
					fmemcpy(begin, str, newlen);
				}
				_buf[size] = 0;
				_size = size;
			}
			return *this;
		}

		inline ftstring& replace(const iterator& begin, const iterator& end, const char* str)
		{
			//return replace(begin._ptr - _buf, end._ptr - begin._ptr, str);

			/// faster than above?
			if (!this->empty() && begin._ptr && begin._ptr <= end._ptr && begin._ptr >= _buf && str != nullptr)
			{
				((iterator&)end)._ptr = end._ptr < _buf + _size ? end._ptr : _buf + _size;

				auto oldlen = (size_t)(end._ptr - begin._ptr);
				auto newlen = std::strlen(str);
				auto size = _size - oldlen + newlen;
				if (size >= Size)
				{
					char error[128];
					std::snprintf(error, 128, "[ftstring::replace]no enough buffer size to replace, %u >= %u, str: %s", size, Size, _buf);
					throw std::runtime_error(error);
				}
				
				auto restlen = _buf + _size - end._ptr;
				if (oldlen >= newlen)
				{
					fmemcpy(begin._ptr, str, newlen);
					if (oldlen > newlen)
					{
						std::memmove(begin._ptr + newlen, end._ptr, restlen);
					}
				}
				else
				{
					std::memmove(end._ptr + newlen - oldlen, end._ptr, restlen);
					fmemcpy(begin._ptr, str, newlen);
				}
				_buf[size] = 0;
				_size = size;
			}
			return *this;
		}

		inline size_t find(const char* str, size_t offset = 0) const
		{
			if (this->empty() || nullptr == str)
				return std::string::npos;

#ifdef USE_SIMD
			auto pos = fast_strfind(_buf + offset, _size - offset, str, strlen(str));
			if (pos != std::string::npos)
				return offset + pos;
			return pos;
#else
			auto ret = std::strstr(_buf + offset, str);
			if (nullptr == ret)
				return std::string::npos;
			return ret - str;
#endif
		}

		inline size_t max_size() const
		{
			return Size;
		}

		inline void clear()
		{
			_size = 0;
			_buf[0] = 0;
		}

		~ftstring()
		{
			clear();
		}

		inline void resize(size_t size)
		{
			if (size >= Size)
			{
				char error[128];
				std::snprintf(error, 128, "[ftstring::resize(%llu)]exceeds the max available size(%llu)", size, Size);
				throw std::runtime_error(error);
			}

			std::memset(_buf + (size < _size ? size : _size), 0, abs((int64)size - (int64)_size) + 1);
			_size = size;
		}

		inline ftstring& append(const char* str, size_t len)
		{
			if (str && len > 0)
			{
				fmemcpy(_buf + _size, str, (uint32_t)(_size + len >= Size ? (Size - _size - 1) : len));
				_size += len;
				_buf[_size] = 0;
			}
			return *this;
		}

		inline ftstring& append(const char* str)
		{
			if (str)
				return append(str, strlen(str));
			return *this;
		}

		inline ftstring& append(const ftstring& str)
		{
			return append(str._buf, str._size);
		}

		inline ftstring& append(const std::string& str)
		{
			return append(str.c_str(), str.size());
		}

		inline bool starts_with(const char* str) const
		{
			if (_size > 0 && str && str != "")
			{
				auto len = std::strlen(str);
				if (len <= _size)
					return 0 == std::memcmp(_buf, str, len);
			}
			return false;
		}

		inline bool ends_with(const char* str) const
		{
			if (_size > 0 && str && str != "")
			{
				auto len = std::strlen(str);
				if (len <= _size)
					return 0 == std::memcmp(_buf + _size - len, str, len);
			}
			return false;
		}

		inline ftstring substr(size_t offset = 0, size_t count = -1) const
		{
			ftstring str;
			if (offset < _size && count > 0)
				str.append(_buf + offset, count < _size - offset ? count : (_size - offset));
			return str;
		}

		operator const std::string() const
		{
			std::string str(_buf, _size);
			return str;
		}

		void reserve() {}
		void reserve(size_t size) {}
	};

	template<size_t Size>
	ftstring<Size> operator+(const char* pstr, const ftstring<Size>& str)
	{
		ftstring<Size> ret;
		ret.append(pstr);
		ret.append(str);
		return ret;
	}

#ifdef USE_PARELLEL
	typedef fstdstring<fast_alloc<char>> stdstring;
#else
	typedef fstdstring<std::allocator<char>> stdstring;
#endif

	typedef ftstring<64> fstring;

	typedef fstring string;
	//typedef stdstring string;

	template<class T = stdstring>
#ifdef WINDOWS
	static
#endif
	T& replace(T& str, const std::string& before, const std::string& after)
	{
		for (std::string::size_type pos(0); pos != std::string::npos; pos += after.length())
		{
			pos = str.find(before, pos);
			if (pos != std::string::npos)
				str.replace(pos, before.length(), after);
			else
				break;
		}
		return str;
	}

#ifdef WINDOWS
	static
#endif
	inline std::string& replace(std::string& str, const std::string& before, const std::string& after)
	{
		return replace<std::string>(str, before, after);
	}
}

static uint32_t bizzard_hash(const char* key)
{
	if (nullptr == key)
	{
		return -1;
	}

	int dwHashType = 1;
	uint32_t seed1 = 0x7FED7FED, seed2 = 0xEEEEEEEE;
	int ch;
	while (*key != 0)
	{
		ch = std::toupper(*key++);

		seed1 = ((dwHashType << 8) + ch) ^ (seed1 + seed2);
		seed2 = ch + seed1 + seed2 + (seed2 << 5) + 3;
	}
	return seed1;
}

namespace std
{
	template<>
	struct hash<taf::common::fstring>
	{
		std::size_t operator()(const taf::common::fstring& key) const
		{
			return bizzard_hash(key.c_str());
			//return std::hash<const char*>()(key.c_str());
		}
	};

	template<>
	struct hash<taf::common::stdstring>
	{
		std::size_t operator()(const taf::common::stdstring& key) const
		{
			return bizzard_hash(key.c_str());
			//return std::hash<const char*>()(key.c_str());
		}
	};
}