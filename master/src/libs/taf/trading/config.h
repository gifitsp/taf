#pragma once

#define INIT_CAPACITY 512

namespace taf
{
	static _force_inline std::shared_ptr < Exchange > get_exchange(const common::string& exchange_name)
	{
		auto& exchange = exchanges()[exchange_name];
		if (!exchange)
		{
			exchange = std::shared_ptr < Exchange >(new Exchange);
			exchange->name = exchange_name;
		}
		return exchange;
	}

	static _force_inline std::shared_ptr < Account > get_account(const common::string& exchange_name, const common::string& account_name)
	{
		auto& account_list = get_exchange(exchange_name)->account_list;
		auto it = account_list.find(account_name);
		if (it == account_list.end())
			throw std::runtime_error("account[" + account_name + "] not found");
		return it->second;
	}

	static _force_inline std::shared_ptr < Instrument > get_instrument(const common::string& exchange_name, const common::string& category, const common::string& symbol)
	{
		auto& instrument_list = get_exchange(exchange_name)->instrument_list;
		auto it = instrument_list.find(category);
		if (it == instrument_list.end())
			throw std::runtime_error("instrument category[" + category + "] not found");
		
		auto& symbols = it->second;
		{
			auto it = symbols.find(symbol);
			if (it == symbols.end())
				throw std::runtime_error("instrument symbol[" + category + "] not found");
			return it->second;
		}
	}

	template<class TradeApi>
	static _force_inline void config_add_tradeapi(const common::string& exchange_name, const common::string& api_name)
	{
		try
		{
			auto& tradeapi_list = get_exchange(exchange_name)->tradeapi_list;
			if (tradeapi_list.find(api_name) == tradeapi_list.end())
				tradeapi_list[api_name] = std::shared_ptr<ITradeApi>(new TradeApi());
		}
		catch (std::exception& e)
		{
			taf::logger().console(taf::Log::Fatal, "config_add_tradeapi[%s][%s] exception: %s\n", exchange_name.c_str(), api_name.c_str(), e.what());
			EXIT_PROCESS(1);
		}
	}

	template<class T, typename...Params>
	static void register_strategy(const taf::common::string& name, Params...params)
	{
		try
		{
			StrategyManager::instance().register_strategy<T>(name, params...);
		}
		catch (std::exception& e)
		{
			taf::logger().console(taf::Log::Fatal, "register_strategy[%s] exception: %s\n", name.c_str(), e.what());
			EXIT_PROCESS(1);
		}
	}

	static std::shared_ptr<IStrategy> get_strategy(const taf::common::string& name)
	{
		return StrategyManager::instance().get_strategy(name);
	}
}

extern "C"
{
	_export void taf_config_add_account(
		const char* exchange_name
		, const char* account_name
		, const char* api_name
		, const char* category
		, const char* asset
		, const char* currency
		, const char* id = "")
	{
		std::shared_ptr<taf::Account> account(new taf::Account);
		account->exchange = exchange_name;
		account->id = id;
		account->api_name = api_name;
		account->category = category;
		account->asset = asset;
		account->currency = currency;
		taf::get_exchange(exchange_name)->account_list[account_name] = account;
	}

	_export void taf_config_add_instrument(
		const char* exchange_name
		, const char* category
		, const char* symbol
		, double price_unit///the minimum unit of price change
		, int price_decimal
		, double min_size///the minimum size accepted by exchange
		, int size_decimal
		, double fee_ratio_taker
		, double fee_ratio_maker)
	{
		std::shared_ptr<taf::Instrument> instrument(new taf::Instrument);
		instrument->category = category;
		instrument->symbol = symbol;
		instrument->price_unit = price_unit;
		instrument->price_decimal = price_decimal;
		instrument->min_size = min_size;
		instrument->size_decimal = size_decimal;
		instrument->fee_ratio_taker = fee_ratio_taker;
		instrument->fee_ratio_maker = fee_ratio_maker;

		auto& instrument_list = taf::get_exchange(exchange_name)->instrument_list;
		instrument_list[category];
		instrument_list[category][symbol] = instrument;
	}

	_export void taf_run_strategy(const char* name)
	{
		taf::StrategyManager::instance().run_strategy(name);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	_export void taf_stop_strategy(const char* name)
	{
		taf::StrategyManager::instance().stop_strategy(name);
	}

	_export void taf_stop_all_strategy()
	{
		taf::StrategyManager::instance().stop_all_strategy();
	}
}