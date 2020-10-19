
#include "../src/libs/taf/frameworks.hpp"


class ExchangeExample1 : public taf::ITradeApi
{
public:
	ExchangeExample1()
	{
		this->set_config("exchange1", "key", "secret", "passcode");
	}

	void init()
	{
		/// apply api-key here
		/// _key;
		/// _secret;
		/// _passcode;
	}

	void post_order(std::shared_ptr<taf::Order>& order)
	{
		//printf("api[%s][post order] side: %s, price: %s, size: %s\n", this->name().c_str(), order->side().c_str(), order->price().c_str(), order->size().c_str());
	}

	void post_orders(const taf::common::stdstring& json)///batch
	{
		printf("api[%s][batch post orders] json: %s\n", this->name().c_str(), json.c_str());
	}

	void cancel_order(std::shared_ptr<taf::Order>& order)
	{
		//printf("api[%s][cancel order] client id: %s, side: %s, price: %s, size: %s\n", this->name().c_str(), order->client_order_id().c_str(), order->side().c_str(), order->price().c_str(), order->size().c_str());
	}

	void cancel_orders(const taf::common::stdstring& json)///batch
	{
		printf("api[%s][batch cancel orders] json: %s\n", this->name().c_str(), json.c_str());
	}

	std::shared_ptr<taf::Order> query_order(const taf::common::string& id)
	{
		printf("### api[%s][query order] client id: %s\n", this->name().c_str(), id.c_str());
		std::shared_ptr<taf::Order> order(new taf::Order());//for test
		order->client_order_id() = id;
		static bool status = false;
		if (status)
		{
			status = false;
			order->filled() = "10";
			order->status() = "filled";
		}
		else
		{
			status = true;
			order->filled() = "1.2";
			order->status() = "partially-filled";
		}
		return order;
	}

	std::shared_ptr<taf::Order> query_orders(const taf::common::stdstring& json)///batch
	{
		printf("api[%s][batch query orders] json: %s\n", this->name().c_str(), json.c_str());
		return std::shared_ptr<taf::Order>();
	}

	void query_balance(taf::Account& account)
	{
		account.asset_balance = 0.1;
		account.currency_balance = 100;
	}
};

class ExchangeExample2 : public ExchangeExample1 ///should derived from taf::ITradeApi here
{
public:
	ExchangeExample2()
	{
		this->set_config("exchange2", "key", "secret", "passcode");
	}

	void init()
	{
		///...
	}

	///...
};

class Strategy1 : public taf::Strategy< Strategy1 >
{
	TradeType exchange1_swap_trade;

	std::shared_ptr<taf::Account> account_swap_btcusd;
	std::shared_ptr<taf::Account> account_swap_btcusdt;
	std::shared_ptr<taf::Instrument> instrument_swap_btcusd;
	std::shared_ptr<taf::Instrument> instrument_swap_btcusdt;

	taf::common::utils::TimeUtil stopwatch;

public:
	Strategy1()
	{
		auto exchange1_futures_worker = create_worker("exchange1", "exchange1_futures_worker", "api1-swap");
		exchange1_swap_trade = exchange1_futures_worker->trade();

		account_swap_btcusd = taf::get_account("exchange1", "btcusd-swap-account");
		account_swap_btcusdt = taf::get_account("exchange1", "btcusdt swap");
		instrument_swap_btcusd = taf::get_instrument("exchange1", "swap", "BTCUSD");
		instrument_swap_btcusdt = taf::get_instrument("exchange1", "swap", "BTCUSDT");
	}

	bool process()
	{
		if (_unlikely(!exchange1_swap_trade->is_running()))
			return false;

		std::shared_ptr<taf::Order> order;
		order.reset(new taf::Order());
		
		taf::common::utils::uid_for_current_process(order->client_order_id());
		order->category() = "swap";
		order->symbol() = "BTCUSD";
		order->side() = "long";//or buy
		order->price() = "10000.0";
		order->size() = "10";
		order->status() = "open";
		exchange1_swap_trade->async_post_order(order);

		{///read only, never change the value in here
			if (0 == account_swap_btcusd->asset_balance) {}
			if (0 == account_swap_btcusd->currency_balance) {}
			if (0 == account_swap_btcusdt->asset_balance) {}
			if (0 == account_swap_btcusdt->currency_balance) {}

			if (0 == instrument_swap_btcusd->min_size) {}
			if (0 == instrument_swap_btcusd->price_decimal) {}
			if (0 == instrument_swap_btcusd->price_unit) {}
			if (0 == instrument_swap_btcusd->size_decimal) {}

			if (0 == instrument_swap_btcusdt->min_size) {}
			if (0 == instrument_swap_btcusdt->price_decimal) {}
			if (0 == instrument_swap_btcusdt->price_unit) {}
			if (0 == instrument_swap_btcusdt->size_decimal) {}
		}

		exchange1_swap_trade->async_cancel_order(order);
		
		auto cost = stopwatch.end_ns();
		taf::logger().console(taf::Log::Debug1, "cost1: %lld\n", cost);
		stopwatch.start_ns();

		return true;
	}

	void _order_filled_changed(std::shared_ptr<taf::Order>& order, double changed)
	{
	}

	void _order_closed(std::shared_ptr<taf::Order>& order)
	{
	}
};

class Strategy2 : public taf::Strategy< Strategy2 >
{
	TradeType exchange1_spot_trade;
	TradeType exchange2_trade;

	std::shared_ptr<taf::Account> exchange1_account_spot_btcusdt;
	std::shared_ptr<taf::Account> exchange2_account_spot_ethusdt;
	std::shared_ptr<taf::Instrument> exchange1_instrument_spot_btcusdt;
	std::shared_ptr<taf::Instrument> exchange2_instrument_spot_ethusdt;

	taf::common::utils::TimeUtil stopwatch;

public:
	Strategy2()
	{
		auto exchange1_spot_worker = create_worker("exchange1", "exchange1_spot_worker", "api1-spot");
		exchange1_spot_trade = exchange1_spot_worker->trade();
		auto exchange2_worker = create_worker("exchange2", "exchange2_worker", "api2");
		exchange2_trade = exchange2_worker->trade();

		exchange1_account_spot_btcusdt = taf::get_account("exchange1", "btcusdt spot account");
		exchange2_account_spot_ethusdt = taf::get_account("exchange2", "ethusdt");
		exchange1_instrument_spot_btcusdt = taf::get_instrument("exchange1", "spot", "BTCUSDT");
		exchange2_instrument_spot_ethusdt = taf::get_instrument("exchange2", "spot", "ETHUSDT");
	}

	bool process()
	{
		auto cost = stopwatch.end_ns();
		taf::logger().console(taf::Log::Debug1, "cost2: %lld\n", cost);
		stopwatch.start_ns();
		return true;
	}
};

class Strategy3 : public taf::Strategy< Strategy3 >
{
	TradeType _trade;

	std::shared_ptr<taf::Account> _account;
	std::shared_ptr<taf::Instrument> _instrument;

	taf::common::utils::TimeUtil stopwatch;

public:
	Strategy3(const taf::common::string& exchange_name
		, const taf::common::string& api_name
		, const taf::common::string& account_name
		, const taf::common::string& category
		, const taf::common::string& symbol)
	{
		auto worker = create_worker(exchange_name, account_name, api_name);
		_trade = worker->trade();
		
		_account = taf::get_account(exchange_name, account_name);
		_instrument = taf::get_instrument(exchange_name, category, symbol);
	}

	bool process()
	{
		auto cost = stopwatch.end_ns();
		taf::logger().console(taf::Log::Debug1, "cost3[%s]: %lld\n", _account->asset.c_str(), cost);
		stopwatch.start_ns();
		return true;
	}
};

void run(bool init, bool load_config)
{
	if (init)
	{///only run once
		if (load_config)
		{///don't execute this code snippet if use Python or other language to add config
			taf_config_add_account("exchange1", "btcusdt spot account", "api1-spot", "spot", "btc", "usdt");
			taf_config_add_account("exchange1", "ltcusdt spot account", "api1-spot", "spot", "ltc", "usdt");
			taf_config_add_account("exchange1", "btcusd-swap-account", "api1-swap", "swap", "btc", "btc");
			taf_config_add_account("exchange1", "btcusdt swap", "api1-swap", "swap", "btc", "usdt");
			taf_config_add_account("exchange2", "ethusdt", "api2", "spot", "eth", "usdt");

			taf_config_add_instrument("exchange1", "spot", "BTCUSDT", 0.01, 2, 0.001, 3, 0.002, -0.001);
			taf_config_add_instrument("exchange1", "spot", "LTCUSDT", 0.01, 2, 0.001, 3, 0.002, -0.001);
			taf_config_add_instrument("exchange1", "swap", "BTCUSD", 0.1, 1, 0.1, 0, -0.0001, -0.0002);
			taf_config_add_instrument("exchange1", "swap", "BTCUSDT", 0.01, 2, 0.001, 3, -0.0001, -0.0002);
			taf_config_add_instrument("exchange2", "spot", "ETHUSDT", 0.01, 2, 0.01, 2, 0.0002, -0.0001);
		}

		taf::config_add_tradeapi<ExchangeExample1>("exchange1", "api1-spot");
		taf::config_add_tradeapi<ExchangeExample1>("exchange1", "api1-swap");
		taf::config_add_tradeapi<ExchangeExample2>("exchange2", "api2");

		taf::register_strategy<Strategy1>("Strategy1");
		taf::register_strategy<Strategy2>("Strategy2");
		taf::register_strategy<Strategy3>("Strategy3-1", "exchange1", "api1-spot", "btcusdt spot account", "spot", "BTCUSDT");
		taf::register_strategy<Strategy3>("Strategy3-2", "exchange1", "api1-spot", "ltcusdt spot account", "spot", "LTCUSDT");


		taf::get_exchange("exchange1")->start_query_balance_routine(30);
	}

	taf_run_strategy("Strategy1");
	taf_run_strategy("Strategy2");
	taf_run_strategy("Strategy3-1");
	taf_run_strategy("Strategy3-2");

	std::this_thread::sleep_for(std::chrono::seconds(3));
	taf_stop_strategy("Strategy3-1");
	std::this_thread::sleep_for(std::chrono::seconds(3));
	taf_run_strategy("Strategy3-1");

	std::this_thread::sleep_for(std::chrono::seconds(60));
}

int main()
{
	taf::logger().set_level(taf::Log::Debug1);

	run(true, true);
	bool ret = taf::get_strategy("Strategy1")->is_running();
	taf_stop_all_strategy();
	std::this_thread::sleep_for(std::chrono::seconds(3));
	run(false/*init only once*/, false);
	return 0;
}

