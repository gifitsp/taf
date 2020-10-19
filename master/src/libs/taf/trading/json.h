#pragma once

namespace taf
{
	class StreamJson
	{
		//typedef std::string String;
		typedef taf::common::stdstring String;

		String _json;
		size_t _json_len;
		size_t _offset;
		String _value;
		std::vector<String> _values;

		inline bool _read_char(char c)
		{
			if (c != ',' && c != '}' && c != ']')
			{
				if (c != ' ' && c != '\"')
				{
					_value += c;
				}
			}
			else
			{
				return false;
			}
			return true;
		}

	public:
		typedef std::runtime_error Exception;

		StreamJson()
		{
			//_value.reserve(128);
			//_values.reserve(10);
		}

		void load(const String& value)
		{
			_json = value;

			for (auto it = _json.rbegin(); it != _json.rend(); ++it)
			{
				if (*it != '}' && *it != ' ')
				{
					_json += "}";
					break;
				}
				else
					break;
			}

			_json_len = _json.size();
			_offset = 0;

			_value = "";
			_values.clear();
		}

		inline const String& json_str() const
		{
			return _json;
		}

		String& read_value(const char* key)
		{
			_value = "";
			if (nullptr == key || key == "")
				return _value;

			auto key_len = strlen(key);

			auto pos = _json.find(key, _offset);
			if (String::npos == pos)
				throw Exception((String("key not found: ") + key).c_str());

			_offset = pos + key_len;
			bool read = false;
			for (; _offset < _json_len; ++_offset)
			{
				auto& c = _json[_offset];
				if (read)
				{
					if (!_read_char(c))
						break;
				}
				if (c == ':')
				{
					read = true;
				}
			}

			if (!read)
				throw Exception("json format error");
			return _value;
		}

		typedef void(*VEC_CALLBACK)(const char* key, int index, std::vector<String>& values);
		typedef std::function<void(const char* key, int index, std::vector<String>&)> VEC_FUNCTION;

		void read_vector(const char* key, VEC_FUNCTION vec_callback)
		{
			if (nullptr == key || key == "")
				return;

			auto key_len = strlen(key);

			auto pos = _json.find(key, _offset);
			if (String::npos == pos)
				throw Exception((String("key not found: ") + key).c_str());
			_offset = pos + key_len;

			bool read1 = false;
			bool read2 = false;

			int index = -1;
			for (; _offset < _json_len; ++_offset)
			{
				auto& c = _json[_offset];
				if (c == '[')
				{
					_offset += 1;
					for (; _offset < _json_len; ++_offset)
					{
						auto& c = _json[_offset];
						if (c == '[')
						{
							_values.resize(0);
							++index;
							_value = "";

							_offset += 1;
							for (; _offset < _json_len; ++_offset)
							{
								auto& c = _json[_offset];

								if (!_read_char(c))
								{
									_values.push_back(_value);
									_value = "";
								}
								if (c == ']')
								{
									read2 = true;
									vec_callback(key, index, _values);
									break;
								}
							}
						}
						if (c == ']')
						{
							read1 = true;
							break;
						}
					}
					break;
				}
			}

			if (!read1 || !read2)
				throw Exception("json format error");
		}

		void start_read_vector(const char* key)
		{
			if (nullptr == key || key == "")
				return;

			auto key_len = strlen(key);

			auto pos = _json.find(key, _offset);
			if (String::npos == pos)
				throw Exception((String("key not found: ") + key).c_str());
			_offset = pos + key_len;

			pos = _json.find("[", _offset);
			if (String::npos == pos)
				throw Exception((String("json format error: ") + key).c_str());
			_offset = pos + 1;
		}

		void move_to_value()
		{
			auto pos = _json.find("[", _offset);
			if (String::npos == pos)
				throw Exception("json format error");
			_offset = pos + 1;
		}

		bool move_to_vector_value()
		{
			auto pos1 = _json.find("[", _offset);
			if (String::npos == pos1)
				return false;

			auto pos2 = _json.find("]", _offset);
			if (String::npos != pos2 && pos1 > pos2)
				return false;

			_offset = pos1 + 1;
			return true;
		}

		String& read_value()
		{
			_value = "";
			for (; _offset < _json_len; ++_offset)
			{
				auto& c = _json[_offset];
				if (!_read_char(c))
				{
					break;
				}
			}

			if (_offset < _json_len - 1)
			{
				auto& c = _json[_offset];
				if (c != '}' && c != ']')
					++_offset;
			}
			else
			{
				++_offset;
			}

			return _value;
		}

		void end_read_vector_value()
		{
			auto pos = _json.find("]", _offset);
			if (String::npos != pos)
				_offset = pos + 1;
		}
	};
}