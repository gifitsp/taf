#pragma once

namespace taf::common::utils
{
	/// <summary>
	/// nanoseconds supported
	/// </summary>
	class TimeUtil
	{
	public:
		typedef decltype(std::chrono::system_clock::now()) system_time_type;
		typedef decltype(std::chrono::high_resolution_clock::now()) time_type;

	private:
		typedef std::chrono::nanoseconds nano;
		typedef std::chrono::microseconds micro;
		typedef std::chrono::milliseconds milli;

		uint64 _clock_ns = 0;
		uint64 _clock_us = 0;
		uint64 _clock_ms = 0;

		template<typename T, typename TimeType>
		static inline uint64 _get_clock(const TimeType& time)
		{
			return std::chrono::duration_cast<T>(time.time_since_epoch()).count();
		}

		static inline time_type clock()
		{
			return std::chrono::high_resolution_clock::now();
		}

		template<typename T>
		static inline uint64 get_clock()
		{
			return _get_clock<T>(clock());
		}

	public:
		enum Precision
		{
			Second,
			MilliSecond,
			MicroSecond,
			NanoSecond,
		};

		const static char* _format;

		/// <summary>
		/// unix seconds -> tm struct
		/// </summary>
		/// <param name="unix_sec"></param>
		/// <param name="tm"></param>
		/// <param name="time_zone"></param>
		/// <returns></returns>
		static int local_time(const time_t& unix_sec, struct tm* tm, int time_zone = 8)
		{
			const int kHoursInDay = 24;
			const int kMinutesInHour = 60;
			const int kDaysFromUnixTime = 2472632;
			const int kDaysFromYear = 153;
			const int kMagicUnkonwnFirst = 146097;
			const int kMagicUnkonwnSec = 1461;

			tm->tm_sec = unix_sec % kMinutesInHour;
			int i = (int)(unix_sec / kMinutesInHour);
			tm->tm_min = i % kMinutesInHour; //nn
			i /= kMinutesInHour;
			tm->tm_hour = (i + time_zone) % kHoursInDay; // hh
			tm->tm_mday = (i + time_zone) / kHoursInDay;
			int a = tm->tm_mday + kDaysFromUnixTime;
			int b = (a * 4 + 3) / kMagicUnkonwnFirst;
			int c = (-b * kMagicUnkonwnFirst) / 4 + a;
			int d = ((c * 4 + 3) / kMagicUnkonwnSec);
			int e = -d * kMagicUnkonwnSec;
			e = e / 4 + c;
			int m = (5 * e + 2) / kDaysFromYear;
			tm->tm_mday = -(kDaysFromYear * m + 2) / 5 + e + 1;
			tm->tm_mon = (-m / 10) * 12 + m + 2;
			tm->tm_year = b * 100 + d - 6700 + (m / 10);
			return 0;
		}

		/// <summary>
		/// for stopwatch
		/// </summary>
		/// <returns></returns>
		inline void start_ns()
		{
			_clock_ns = get_clock<nano>();
		}

		inline uint64 end_ns()
		{
			if (0 == _clock_ns)
				return 0;
			return get_clock<nano>() - _clock_ns;
		}

		inline void start_us()
		{
			_clock_us = get_clock<micro>();
		}

		inline uint64 end_us()
		{
			if (0 == _clock_us)
				return 0;
			return get_clock<micro>() - _clock_us;
		}

		inline void start_ms()
		{
			_clock_ms = get_clock<milli>();
		}

		inline uint64 end_ms()
		{
			if (0 == _clock_ms)
				return 0;
			return get_clock<milli>() - _clock_ms;
		}

		static inline system_time_type now()
		{
			return std::chrono::system_clock::now();
		}

		static inline uint64 get_ns_tick()
		{
			return _get_clock<nano>(now());
		}

		static inline uint64 get_us_tick()
		{
			return _get_clock<micro>(now());
		}

		static inline uint64 get_ms_tick()
		{
			return _get_clock<milli>(now());
		}

		static const std::string get_current_time(Precision precision = NanoSecond, const char* base_format = _format)
		{
			auto now = TimeUtil::now();

			std::ostringstream oss;

			auto seconds = _get_clock<std::chrono::seconds>(now);
			auto nanoseconds = _get_clock<nano>(now);
			auto microseconds = nanoseconds / 1000;
			auto milliseconds = microseconds / 1000;

			auto dif_milli = milliseconds % 1000;
			auto dif_micro = microseconds % 1000;
			auto dif_nano = nanoseconds % 1000;

			char format[DEFAULT_SIZE];
			switch (precision)
			{
			case NanoSecond:
				std::snprintf(format, DEFAULT_SIZE, "%s.%3lld.%3lld.%3lld", base_format, dif_milli, dif_micro, dif_nano);
				break;
			case MicroSecond:
				std::snprintf(format, DEFAULT_SIZE, "%s.%3lld.%3lld", base_format, dif_milli, dif_micro);
				break;
			case MilliSecond:
				std::snprintf(format, DEFAULT_SIZE, "%s.%3lld", base_format, dif_milli);
				break;
			default:
				std::snprintf(format, DEFAULT_SIZE, "%s", base_format);
				break;
			}

			auto t = std::chrono::system_clock::to_time_t((std::chrono::system_clock::time_point&)now);
			std::tm _tm;
			local_time(t, &_tm);
			oss << std::put_time(&_tm, format);
			return oss.str();
		}

		/// <summary>
		/// return total nanoseconds
		/// input format should be ms.us.nan
		/// </summary>
		/// <param name="time"></param>
		/// <param name="format"></param>
		/// <returns></returns>
		static uint64 from_string(const char* time, const char* format = _format)
		{
			std::tm tm;
			std::stringstream ss(time);
			ss >> std::get_time(&tm, format);
			auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));

			uint64 nanoseconds = _get_clock<std::chrono::nanoseconds, system_time_type>(tp);
			auto p = std::strstr(time, ".");
			int i = 0;
			const int MULTIS[] = { 1000'000, 1000, 1 };
			while (p != nullptr && *p != '\0' && *(++p) != '\0')
			{
				nanoseconds += std::strtol(p, (char**)&p, 0) * MULTIS[i++];
			}

			return nanoseconds;
		}

		static inline uint64 from_string(const std::string& time, const char* format = _format)
		{
			return from_string(time.c_str(), format);
		}

		static inline uint64 from_utc_string(const char* time, const char* format = "%Y-%m-%dT%T")
		{
			return from_string(time, format);
		}

		static inline uint64 from_utc_string(const std::string& time, const char* format = "%Y-%m-%dT%T")
		{
			return from_string(time.c_str(), format);
		}
	};

	/// <summary>
	/// format based on std::chrono
	/// </summary>
	const char* TimeUtil::_format = "%Y-%m-%d %T";

#ifdef _MSC_VER
	static inline uint64_t rte_rdtsc(void)
	{
		return TimeUtil::get_ns_tick();
	}
#else
	static inline uint64_t rte_rdtsc(void)
	{
		union {
			uint64_t tsc_64;
			struct {
				uint32_t lo_32;
				uint32_t hi_32;
			};
		} tsc;

		asm volatile("rdtsc" :
					 "=a" (tsc.lo_32),
					 "=d" (tsc.hi_32));
		return tsc.tsc_64;
	}
#endif

	/// <summary>
	/// simple way to gerate uid for current process
	/// String must be compatible with std::string
	/// </summary>
	/// <typeparam name="String"></typeparam>
	/// <returns></returns>
	template<typename String>
	static inline void uid_for_current_process(String& struid, std::atomic<uint64_t>& uid, const char* prefix = "", const char* suffix = "")
	{
		uid.fetch_add(1);
		struid = std::to_string(uid);// + "-" + std::to_string(rte_rdtsc());
		if (prefix != "")
			struid = prefix + ("-" + struid);
		if (suffix != "")
			struid = struid + "-" + suffix;
	}

	template<typename String>
	static inline void uid_for_current_process(String& struid, const char* prefix = "", const char* suffix="")
	{
		static std::atomic<uint64_t> uid(0);
		uid_for_current_process(struid, uid, prefix, suffix);
	}
}