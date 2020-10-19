#pragma once

#include <vector>

namespace taf::common::utils
{
	template<size_t Size>
	class Buffers
	{
		struct Buf
		{
			char* buf;
			Buf()
			{
				buf = new char[Size];
			}
			Buf(char* p)
			{
				buf = p;
			}
		};

		taf::common::utils::LockFreeQueue<Buf> _buffers;
		size_t _size;

	public:
		Buffers(size_t size = 64)
		{
			_size = size * 2;
			_buffers.reserve(_size);

			for (int i = 0; i < _size; ++i)
				_buffers.push(Buf());
		}

		inline void* pop()
		{
			Buf buf(nullptr);
			if (_buffers.pop(buf))
			{
				return buf.buf;
			}
			return new char[Size];
		}

		inline void push(void* p)
		{
			if (_buffers.size() < _size)
				_buffers.push(Buf((char*)p));
			else
				delete[] (char*)p;
		}
	};
}