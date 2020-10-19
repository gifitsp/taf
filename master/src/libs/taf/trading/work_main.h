#pragma once

namespace taf
{
	template<class Processor, class Worker>
	class MainWorker : public Work<MainWorker<Processor, Worker>>
	{
		typedef Work<MainWorker<Processor, Worker>> Base;

		Processor& _processor;
		OrderEventData _data;

	public:
		std::vector<std::shared_ptr<Worker>> workers;

	public:
		MainWorker(IStrategy& strategy, Processor& processor, TaskQueues& queues, common::utils::Event& work_event)
			: Base(strategy, "MainWorker", queues, std::shared_ptr<ITradeApi>(), work_event)
			, _processor(processor)
		{
		}

		_force_inline void _before_run()
		{
			Base::_set_work_thread_id = true;
		}

		_force_inline void _ready_to_run()
		{
			for (auto worker : workers)
			{
				worker->order_worker->set_work_thread_id(Base::_work_thread_id);
				worker->trade_worker->set_work_thread_id(Base::_work_thread_id);
			}
		}

		_force_inline void _after_run()
		{
		}

		void _work()
		{
			int count = 0;
			bool default_mode = _processor.process();
			if (!default_mode)
				Base::_work_event.wait();

			auto handle_event = [&]()
			{
				switch (_data.order_event)
				{
				case event_update_order:
				{
					auto& order = _data.order;
					_data.order_worker->update_order(order);

					if (_processor.order_event())
					{
						if (!order->filled().empty() && order->filled() != "0")
						{
							auto filled = common::utils::to_type<double>(order->filled().c_str());
							auto last_filled = common::utils::to_type<double>(order->last_filled().c_str());
							if (filled > 0 && filled > last_filled)
							{
								_processor._order_filled_changed(order, filled - last_filled);
								order->last_filled() = order->filled();
							}
						}
					}

					if (order->status() == "filled" || order->status() == "canceled")
					{
						if (_processor.order_event())
							_processor._order_closed(order);

						order->filled() = "";
						order->last_filled() = "";
						_data.order_worker->remove_order(order->client_order_id());
					}
					break;
				}
				case event_remove_order:
					_data.order_worker->remove_order(_data.id);
					break;
				case event_get_order_ids:
					_data.order_worker->get_all_order_ids();
					break;
				}
			};

			while (!default_mode || ++count < 2)
			{
				auto process_event = [&](OrderEventQueue& queue)
				{
					if (queue.pop(_data))
					{
						handle_event();
						return true;
					}
					return false;
				};

				if (!process_event(Base::_queues.order_queues[2]))
					if (!process_event(Base::_queues.order_queues[1]))
						if (!process_event(Base::_queues.order_queues[0]))
							break;
			}
		}
	};
}