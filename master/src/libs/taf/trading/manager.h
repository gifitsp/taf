#pragma once

namespace taf
{
	class IStrategy
	{
	protected:
		volatile bool ORDER_EVENT = true;

	public:
		common::string name;

		_force_inline bool order_event() const
		{
			return ORDER_EVENT;
		}

		virtual void run() = 0;
		virtual void stop() = 0;
		virtual bool is_running() const = 0;

		virtual void _order_filled_changed(std::shared_ptr<Order>& order, double changed) {}
		virtual void _order_closed(std::shared_ptr<Order>& order) {}///filled or canceled
	};

	class StrategyManager
	{
		std::unordered_map<common::string, std::shared_ptr<IStrategy>> _strategies;

		StrategyManager()
		{
			logger().console(Log::Info, "core threads number on this machine: %d\n", parellel::get_core_threads_num());
		}

		~StrategyManager()
		{
			stop_all_strategy();
		}

	public:
		static StrategyManager& instance()
		{
			static StrategyManager sm;
			return sm;
		}

		template<class T, typename...Params>
		void register_strategy(const common::string& name, Params...params)
		{
			if (_strategies.find(name) == _strategies.end())
				_strategies[name] = std::shared_ptr<IStrategy>(new T(params...));
		}

		void run_strategy(const common::string& name)
		{
			auto it = _strategies.find(name);
			if (it != _strategies.end() && !it->second->is_running())
			{
				try
				{
					it->second->name = name;
					it->second->run();
					logger().console(Log::Info, "strategy[%s] is running\n", name.c_str());
				}
				catch (std::exception& e)
				{
					logger().console(Log::Info, "strategy[%s] exception: %s\n", name.c_str(), e.what());
				}
			}
		}

		_force_inline std::shared_ptr<IStrategy> get_strategy(const common::string& name)
		{
			auto it = _strategies.find(name);
			if (it != _strategies.end())
				return it->second;
			return std::shared_ptr<IStrategy>();
		}

		void stop_strategy(const common::string& name)
		{
			auto it = _strategies.find(name);
			if (it != _strategies.end())
			{
				it->second->stop();
			}
		}

		void stop_all_strategy()
		{
			for (auto kv : _strategies)
				stop_strategy(kv.first);
		}
	};
}