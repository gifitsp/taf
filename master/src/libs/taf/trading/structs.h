#pragma once

#include <unordered_map>

#define ORDER_DECLARE_MEMBER(member, index)\
		_force_inline common::string& member()\
		{\
			return *(common::string*)(_buf.data() + index * sizeof(common::string));\
		}

namespace taf
{
	typedef common::utils::Buffers<2048> Buffers;

	struct Order
	{
		Order()
		{
			error_code() = "0";
		}

		//void* operator new(size_t size)
		//{
		//	return _buffers.pop();
		//}

		//void operator delete(void* p)
		//{
		//	_buffers.push(p);
		//}

		ORDER_DECLARE_MEMBER(client_order_id, 0);
		ORDER_DECLARE_MEMBER(order_id, 1);
		ORDER_DECLARE_MEMBER(category, 2); //spot, margin, futures, swap, options
		ORDER_DECLARE_MEMBER(status, 3); //open, partial-filled, filled, canceled
		ORDER_DECLARE_MEMBER(symbol, 4); //combination of asset and currency
		ORDER_DECLARE_MEMBER(price, 5);
		ORDER_DECLARE_MEMBER(avgprice, 6);
		ORDER_DECLARE_MEMBER(size, 7);
		ORDER_DECLARE_MEMBER(side, 8); //buy, sell / long, short
		ORDER_DECLARE_MEMBER(filled, 9);
		ORDER_DECLARE_MEMBER(fee, 10);
		ORDER_DECLARE_MEMBER(executed_value, 11);
		ORDER_DECLARE_MEMBER(last_filled, 12);
		ORDER_DECLARE_MEMBER(type, 13); //sub class / call, put
		ORDER_DECLARE_MEMBER(strik_price, 14); //for options
		ORDER_DECLARE_MEMBER(expires, 15); //for options
		ORDER_DECLARE_MEMBER(create_time, 16);
		ORDER_DECLARE_MEMBER(update_time, 17);
		ORDER_DECLARE_MEMBER(error_code, 18); //0: no error
		common::stdstring error_message;

	private:
		common::ftstring<1024> _buf;
		static Buffers _buffers;
	};
	Buffers Order::_buffers(512);

	struct Account
	{
		common::string id;
		common::string api_name;
		common::string exchange;
		common::string category; //same as Order::category
		common::string asset;
		common::string currency;
		std::atomic<double> asset_balance = 0;
		std::atomic<double> currency_balance = 0;
	};

	struct Instrument
	{
		common::string symbol;
		common::string category; //same as Order::category
		double price_unit = 0;
		int price_decimal = 0;
		double min_size = 0;
		int size_decimal = 0;
		double fee_ratio_taker = 0;
		double fee_ratio_maker = 0;
	};
}