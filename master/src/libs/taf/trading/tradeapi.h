#pragma once

namespace taf
{
	class ITradeApi
	{
	protected:
		common::string _name;
		common::stdstring _key;
		common::stdstring _secret;
		common::stdstring _passcode;

	public:
		void set_config(const common::stdstring& name, const common::stdstring& key, const common::stdstring& secret, const common::stdstring& passcode = "")
		{
			_name = name;
			_key = key;
			_secret = secret;
			_passcode = passcode;
		}

		void _check_data()
		{
			if (_name.empty())
				throw std::runtime_error("api name could not be empty");
			if (_key.empty() || _secret.empty())
				throw std::runtime_error(_name + ": key or secret not set");
		}

		_force_inline auto name() const
		{
			return _name;
		}

		virtual void init() = 0;
		virtual void post_order(std::shared_ptr<Order>& order) = 0;
		virtual void post_orders(const common::stdstring& json) {}
		virtual void cancel_order(std::shared_ptr<taf::Order>& order) = 0;
		virtual void cancel_orders(const common::stdstring& json) {}
		virtual std::shared_ptr<Order> query_order(const common::string& id) = 0;

		virtual std::shared_ptr<Order> query_orders(const common::stdstring& json) { return std::shared_ptr<taf::Order>(); }
		virtual void query_balance(Account& account) {}
	};
}