#pragma once

namespace taf
{
	class Log
	{
	public:
		enum Level
		{
			Fatal,
			Error,
			Warning,
			Info,
			Debug1,
			Debug2,
			Debug3,
			Debug4,
			Debug5,
		};

	protected:
		Level _level;

	public:
		Log()
		{
			_level = Info;
		}

		void set_level(Level level)
		{
			_level = level;
		}
	};

	class SimpleLog : public Log
	{
	public:
		inline void console(Level level, const char* format, ...)
		{
			if (level <= _level)
			{
				auto time = common::utils::TimeUtil::get_current_time();

				std::string strformat;
				if (level <= Warning)
					strformat = time + "|>>>!!! " + format;
				else
					strformat = time + "|>>> " + format;

				va_list arg_list;
				va_start(arg_list, format);
				::vprintf(strformat.c_str(), arg_list);
				va_end(arg_list);
			}
		}
	};

	typedef SimpleLog Logger;

	static Logger& logger()
	{
		static Logger _logger;
		return _logger;
	}
}