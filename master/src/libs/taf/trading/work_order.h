#pragma once

namespace taf
{
	class OrderWorker : public Work<OrderWorker>
	{
		typedef Work<OrderWorker> Base;

		uint32_t _query_order_sleep_second;

		mutable common::utils::Event _get_id_event;
		common::utils::Event _sleep_event;
		std::unordered_map<common::string, std::shared_ptr<Order>> _orders; //<client_order_id, order>
		mutable decltype(_orders) _pass_orders;

		_force_inline void _update_order(const std::shared_ptr<Order>& order)
		{
			_orders[order->client_order_id()] = order;
		}

	public:
		std::shared_ptr<OrderWorker>* this_shared_ptr;

	public:
		OrderWorker(IStrategy& strategy, const common::string& work_name, TaskQueues& queues, std::shared_ptr<ITradeApi>& tradeapi, common::utils::Event& work_event, uint32 query_order_sleep_seconds)
			: Work(strategy, work_name + ": OrderWorker", queues, tradeapi, work_event)
			, _query_order_sleep_second(query_order_sleep_seconds)
		{
			_orders.reserve(INIT_CAPACITY);
			_pass_orders.reserve(INIT_CAPACITY);
		}

		~OrderWorker()
		{
			stop();
		}

		_force_inline void stop()
		{
			Base::stop();
			_get_id_event.notify();
			_sleep_event.notify();

			_orders.clear();
		}

		void _work()
		{
			if (Base::is_running())
			{///query order
				Base::_queues.order_queues[0].push({ *this_shared_ptr, event_get_order_ids });
				Base::_work_event.notify();

				std::shared_ptr<Order> order;

				if (Base::is_running())
				{
					_get_id_event.wait();
					for (auto kv : _pass_orders)
					{
						if (!Base::is_running())
							break;
						order = Base::_tradeapi->query_order(kv.first);
						async_update_order(order);
						if (!Base::is_running())
							break;
						_sleep_event.wait_for(std::chrono::milliseconds(1));
					}
					_pass_orders.clear();
				}
			}

			auto start = common::utils::TimeUtil::get_ms_tick();
			while (!_exit)
			{
				//_sleep_event.wait_for(std::chrono::seconds(_query_order_sleep_second));
				_sleep_event.wait_for(std::chrono::milliseconds(100));
				if ((common::utils::TimeUtil::get_ms_tick() - start) / 1000 >= _query_order_sleep_second)
					break;
			}
		}

		_force_inline void _after_run()
		{
			_pass_orders.clear();
		}

		_force_inline void get_all_order_ids() const
		{
			if (_unlikely(this->_exit))
				return;
			Base::_check_thread("get_all_order_ids");

			auto _get = [&]()
			{
				if (Base::is_running())
					_pass_orders = _orders;
				else
					_pass_orders.clear();
			};
			_get_id_event.lock_and_notify(_get);
		}

		_force_inline void update_order(const std::shared_ptr<Order>& order)
		{
			if (_unlikely(this->_exit))
				return;
			Base::_check_thread("update_order");
			_update_order(order);
		}

		_force_inline std::shared_ptr<Order> get_order(const common::string& client_order_id)
		{
			if (_likely(!this->_exit))
			{
				Base::_check_thread("get_order");

				auto it = _orders.find(client_order_id);
				if (_likely(it != _orders.end()))
					return it->second;
			}
			return std::shared_ptr<Order>();
		}

		_force_inline void remove_order(const common::string& client_order_id)
		{
			if (_unlikely(this->_exit))
				return;
			Base::_check_thread("remove_order");
			_orders.erase(client_order_id);
		}

		inline void async_update_order(const std::shared_ptr<Order>& order)
		{
			Base::_queues.order_queues[0].push({ *this_shared_ptr, event_update_order, "", order });
			Base::_work_event.notify();
		}

		inline void async_remove_order(const common::string& client_order_id)
		{
			Base::_queues.order_queues[0].push({ *this_shared_ptr, event_remove_order, client_order_id });
			Base::_work_event.notify();
		}
	};
}