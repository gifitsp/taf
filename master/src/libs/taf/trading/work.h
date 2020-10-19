#pragma once

namespace taf
{
	enum WorkEvent
	{
		event_nothing,
		event_data,
		event_update_order,
		event_remove_order,
		event_get_order_ids,
	};

	class OrderWorker;

	struct OrderEventData
	{
		std::shared_ptr<OrderWorker> order_worker;
		WorkEvent order_event = event_nothing;
		common::string id;
		std::shared_ptr<Order> order;
		common::stdstring data;
	};

	typedef common::utils::LockFreeQueue<OrderEventData> OrderEventQueue;

	class TaskQueues
	{
	public:
		const static int QUEUE_SIZE = 3;

		/// <summary>
		/// priority set, index 0 has the lowest priority
		/// </summary>
		OrderEventQueue order_queues[QUEUE_SIZE];

		TaskQueues()
		{
			for (int i = 0; i < QUEUE_SIZE; ++i)
			{
				order_queues[i].reserve(INIT_CAPACITY);
			}
		}

		void clear()
		{
			OrderEventData data;
			for (int i = 0; i < QUEUE_SIZE; ++i)
			{
				while (order_queues[i].pop(data));
			}
		}
	};

	template<class T>
	class Work : public std::enable_shared_from_this<Work<T>>
	{
		typedef std::enable_shared_from_this<Work<T>> Base;

	protected:
		IStrategy& _strategy;
		common::string _work_name;
		TaskQueues& _queues;
		std::shared_ptr<ITradeApi> _tradeapi;

		volatile bool _exit = true;
		common::utils::Event& _work_event;

		volatile bool _set_work_thread_id = false;
		std::thread::id _work_thread_id;

		_force_inline void _check_thread(const std::string& function) const
		{
			if (_unlikely(std::this_thread::get_id() != _work_thread_id))
				throw std::runtime_error(std::string(function) + ": wrong thread used. please use MainWorker::_work thread to call again");
		}

		_force_inline bool _can_run() const
		{
			return _work_thread_id != std::thread::id();
		}

		_force_inline void _before_run() {}
		_force_inline void _ready_to_run() {}
		_force_inline void _after_run() {}

	public:
		Work(IStrategy& strategy, const common::string& work_name, TaskQueues& queues, const std::shared_ptr<ITradeApi>& tradeapi, common::utils::Event& work_event)
			: _strategy(strategy)
			, _work_name(work_name)
			, _queues(queues)
			, _tradeapi(tradeapi)
			, _work_event(work_event)
		{
		}

		virtual ~Work() noexcept
		{
			stop();
		}

		_force_inline std::shared_ptr<T> get_shared_this_ptr()
		{
			return (std::shared_ptr<T>&)Base::shared_from_this();
		}

		_force_inline void set_work_thread_id(const std::thread::id& work_thread_id)
		{
			_work_thread_id = work_thread_id;
		}

		_force_inline void stop()
		{
			_work_thread_id = std::thread::id();

			_exit = true;
			_work_event.notify();
		}

		_force_inline auto get_work_thread_id() const
		{
			return _work_thread_id;
		}

		_force_inline bool is_running() const
		{
			return !_exit;
		}

		_force_inline void set_running()
		{
			_exit = false;
		}

		_force_inline common::string full_name()
		{
			return "[" + _strategy.name + "][" + _work_name + "]";
		}

		void work_task()
		{
			set_running();
			bool exception = false;

			try
			{
				((T*)this)->_before_run();
				if (_set_work_thread_id)
					_work_thread_id = std::this_thread::get_id();
				while (!_can_run() && !_exit)
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
				((T*)this)->_ready_to_run();

				while (!_exit)
				{
					((T*)this)->_work();
				}
			}
			catch (std::exception& e)
			{
				exception = true;
				logger().console(taf::Log::Error, "Worker[%s] exception: %s\n", ((T*)this)->full_name().c_str(), e.what());
			}

			((T*)this)->_after_run();
			if (exception)
				_strategy.stop();
		}
	};
}