#ifndef __ONLINEGAME_GS_PLAYER_TRADE_H__
#define __ONLINEGAME_GS_PLAYER_TRADE_H__

/**
 *	这个类保存在玩家间交易时需要保存的一些状态和处理内容
 */

#include <amemobj.h>
#include "item_list.h"
#include "config.h"

class gplayer_imp;
class player_trade  : public abase::ASmallObject
{
	int 	  _time_out;
	int 	  _money;
	item_list _backpack;
	int 	  _trade_id;
public:
	explicit player_trade(int trade_id):_time_out(-1),_money(0),
					    _backpack(item::TEMP_INV,20),_trade_id(trade_id)
	{}
	virtual ~player_trade(){}

	void SetTimeOut(int t)
	{
		_time_out = t;
	}

	int PickupItem(item_data * pData)
	{
		return 	_backpack.Push(*pData);
	}

	void PickupMoney(size_t money)
	{
		_money += money;
	}

	size_t GetMoney() { return _money;}
	item_list & GetBackpack() { return _backpack;}

	int GetTradeID()
	{
		return _trade_id;
	}
	

	//只有取消状态才会调用心跳函数
	bool Heartbeat(gplayer_imp * pImp)
	{	
		return (--_time_out > 0);
	}
};

class faction_trade : public player_trade
{
public:
	int _get_mask;		//超时时取什么内容 
	int _put_mask;		//超时后再次超时将什么数据存盘
	explicit faction_trade(int trade_id,int get_mask, int put_mask):player_trade(trade_id)
	{
		_get_mask = get_mask;
		_put_mask = put_mask;
	}
};
#endif

