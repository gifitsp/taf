#pragma once

namespace taf
{
	template<class Processor>
	class Strategy : public IStrategy
	{
	protected:
		typedef std::shared_ptr<TradeWorker<OrderWorker>> TradeType;

		struct Worker
		{
			std::shared_ptr<OrderWorker> order_worker;
			TradeType trade_worker;

			_force_inline TradeType& trade()
			{
				return trade_worker;
			}

			void work_task()
			{
				trade_worker->set_running();
				order_worker->work_task();
			}

			void stop()
			{
				trade_worker->stop();
				order_worker->stop();
			}

			Worker(IStrategy& strategy, const common::string& work_name, TaskQueues& queues, std::shared_ptr<ITradeApi>& tradeapi, common::utils::Event& work_event, uint32 query_order_sleep_seconds)
			{
				order_worker.reset(new OrderWorker(strategy, work_name, queues, tradeapi, work_event, query_order_sleep_seconds));
				order_worker->this_shared_ptr = &order_worker;
				trade_worker.reset(new TradeWorker<OrderWorker>(strategy, work_name, queues, tradeapi, work_event, order_worker));
			}
		};

		ExchangeList& _exchange_list;
		TaskQueues _task_queues;
		common::utils::Event _work_event;
		MainWorker<Processor, Worker> _main_worker;

		taf::parellel::Tasks _tasks;
		taf::parellel::ThreadManager _thread_manager;

		/// <summary>
		/// key is worker name specified by user
		/// </summary>
		std::unordered_map<common::string, std::shared_ptr<Worker>> _workers;

		std::shared_ptr<Worker> create_worker(const common::string& exchange_name, const common::string& worker_name, const common::string& api_name, uint32 query_order_sleep_seconds = 30)
		{
			auto it_exchange = _exchange_list.find(exchange_name);
			if (it_exchange == _exchange_list.end())
				throw std::runtime_error(common::string("create_worker: ") + exchange_name + " not found");

			auto it = it_exchange->second->tradeapi_list.find(api_name);
			if (it == it_exchange->second->tradeapi_list.end())
				throw std::runtime_error(common::string("api_name[") + api_name + "] is not found");
			
			auto worker = std::shared_ptr<Worker>(new Worker(*this, worker_name, _task_queues, it->second, _work_event, query_order_sleep_seconds));
			_workers[worker_name] = worker;
			return worker;
		}

		_force_inline void call_next()
		{
			_work_event.notify();
		}

	public:
		Strategy()
			: _exchange_list(exchanges())
			, _main_worker(*this, *(Processor*)this, _task_queues, _work_event)
		{
		}

		bool is_running() const
		{
			return _main_worker.is_running();
		}

		void run()
		{
			_main_worker.workers.clear();
			for (auto kv : _workers)
			{
				_main_worker.workers.push_back(kv.second);
				_thread_manager.run(std::bind(&Worker::work_task, &*kv.second));
			}
			_thread_manager.run(std::bind(&MainWorker<Processor, Worker>::work_task, &_main_worker));
		}

		void stop()
		{
			bool is_running = _main_worker.is_running();
			_main_worker.stop();
			for (auto kv : _workers)
				kv.second->stop();
			_task_queues.clear();

			_tasks.cancel();
			_tasks.wait();
			_thread_manager.wait();

			if (is_running)
				logger().console(Log::Info, "strategy[%s] stopped\n", name.c_str());
		}
	};
}