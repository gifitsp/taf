#pragma once

#include <future>

namespace taf
{
	template<class OrderWorker>
	class TradeWorker : public Work<TradeWorker<OrderWorker>>
	{
		typedef Work<TradeWorker<OrderWorker>> Base;
		typedef TradeWorker<OrderWorker> Self;

		std::shared_ptr<OrderWorker> _order_worker;
		taf::parellel::Tasks _tasks;

		_force_inline void _add_order(const std::shared_ptr<Order>& order)
		{
			_order_worker->update_order(order);
		}

		_force_inline void _add_order(const common::string& id)
		{
			std::shared_ptr<Order> order(new Order);
			order->client_order_id = id;
			_add_order(order);
		}

		inline void _add_orders(const std::vector<common::string>& ids)
		{
			auto size = ids.size();
			for (int i = 0; i < size; ++i)
			{
				std::shared_ptr<Order> order(new Order);
				order->client_order_id = ids[i];
				_add_order(order);
			}
		}

		_force_inline void _post_order_thread(std::shared_ptr<Order> order)
		{
			this->Base::_tradeapi->post_order(order);
		}

		_force_inline void _post_orders_thread(const common::stdstring json)
		{
			this->Base::_tradeapi->post_orders(json);
		}

		_force_inline void _cancel_order_thread(std::shared_ptr<Order> order)
		{
			this->Base::_tradeapi->cancel_order(order);
		};

		_force_inline void _cancel_orders_thread(std::shared_ptr<Order> order)
		{
			this->Base::_tradeapi->cancel_orders(order);
		};

	public:
		TradeWorker(IStrategy& strategy, const common::string& work_name, TaskQueues& queues, std::shared_ptr<ITradeApi>& tradeapi, common::utils::Event& work_event, std::shared_ptr<OrderWorker>& order_worker)
			: Base(strategy, work_name + ": TradeWorker", queues, tradeapi, work_event)
			, _order_worker(order_worker)
		{
		}

		~TradeWorker() noexcept
		{
			_tasks.wait();
		}

		void _work()
		{
			Base::_work_event.wait();
		}

		_force_inline void post_order(std::shared_ptr<Order>& order)
		{
			if (_unlikely(this->_exit))
				return;
			Base::_check_thread("post_order");
			_order_worker->update_order(order);
			Base::_tradeapi->post_order(order);
		}

		_force_inline void post_orders(const std::vector<common::string>& ids, const common::stdstring& json)
		{
			if (_unlikely(this->_exit))
				return;
			Base::_check_thread("post_orders by id");
			_add_orders(ids);
			Base::_tradeapi->post_order(json);
		}

		_force_inline void cancel_order(std::shared_ptr<Order>& order)
		{
			Base::_tradeapi->cancel_order(order);
		}

		_force_inline void cancel_orders(const common::stdstring& json)
		{
			Base::_tradeapi->cancel_orders(json);
		}

		_force_inline void async_post_order(std::shared_ptr<Order>& order)
		{
			if (_unlikely(this->_exit))
				return;
			Base::_check_thread("async_post_order");
			_add_order(order);
			_tasks.run(std::bind(&Self::_post_order_thread, this, order));
		}

		_force_inline void async_post_orders(const std::vector<common::string>& ids, const common::stdstring& json)
		{
			if (_unlikely(this->_exit))
				return;
			Base::_check_thread("async_post_orders");
			_add_orders(ids);
			_tasks.run(std::bind(&Self::_post_orders_thread, this, json));
		}

		_force_inline void async_cancel_order(std::shared_ptr<Order>& order)
		{
			_tasks.run(std::bind(&Self::_cancel_order_thread, this, order));
		}

		_force_inline void async_cancel_orders(const common::stdstring& json)
		{
			_tasks.run(std::bind(&Self::_cancel_orders_thread, this, json));
		}
	};
}