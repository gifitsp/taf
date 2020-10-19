#pragma once

namespace taf
{
	class Exchange
	{
	public:
		Exchange()
		{
			for (auto kv : tradeapi_list)
			{
				kv.second->_check_data();
				kv.second->init();
			}
		}

		void start_query_balance_routine(uint32 sleep_seconds = 20)
		{
			auto _thread = [&]()
			{
				while (true)
				{
					for (auto kv : account_list)
					{
						auto it = tradeapi_list.find(kv.second->api_name);
						if (it != tradeapi_list.end())
						{
							it->second->query_balance(*kv.second);
						}
					}
					std::this_thread::sleep_for(std::chrono::seconds(sleep_seconds));
				}
			};

			std::thread t(_thread);
			t.detach();
		}

		/// <summary>
		/// exchange name
		/// </summary>
		common::string name;

		/// <summary>
		/// key: api name from config
		/// </summary>
		std::unordered_map<common::string, std::shared_ptr<ITradeApi>> tradeapi_list;
		/// <summary>
		/// key: account name from config
		/// </summary>
		std::unordered_map<common::string, std::shared_ptr<Account>> account_list;
		/// <summary>
		/// <category, <symbol, >>
		/// </summary>
		std::unordered_map<common::string, std::unordered_map<common::string, std::shared_ptr<Instrument>>> instrument_list;
	};

	typedef std::unordered_map < common::string, std::shared_ptr < Exchange > > ExchangeList;

	static _force_inline ExchangeList& exchanges()
	{
		static ExchangeList _exchanges;
		return _exchanges;
	}
}