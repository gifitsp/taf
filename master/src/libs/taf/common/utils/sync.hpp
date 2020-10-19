#pragma once

namespace taf::common::utils
{
	class Event
	{
		std::mutex _mutex;
		std::condition_variable _cv_event;

	public:
		/// <summary>
		/// F must have no argument
		/// notify: 0: notify_all, otherwise: notify_one
		/// </summary>
		template<typename F>
		inline void lock_and_notify(F& func, int notify = 0)
		{
			std::unique_lock<std::mutex> lck(_mutex);
			func();
			0 == notify ? _cv_event.notify_all() : _cv_event.notify_one();
		}

		inline void notify()
		{
			std::unique_lock<std::mutex> lck(_mutex);
			_cv_event.notify_all();
		}

		inline void wait()
		{
			std::unique_lock<std::mutex> lck(_mutex);
			_cv_event.wait(lck);
		}

		template<typename Duration>
		inline bool wait_for(Duration d)
		{
			std::unique_lock<std::mutex> lck(_mutex);
			if (_cv_event.wait_for(lck, d) == std::cv_status::timeout)
				return false;
			return true;
		}
	};
}