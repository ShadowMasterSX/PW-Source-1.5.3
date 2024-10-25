#ifndef __ONLINEGAME_GAME_GS_PLAYER_STALL_OBJ_H__
#define __ONLINEGAME_GAME_GS_PLAYER_STALL_OBJ_H__

#include <vector.h>
#include "item_list.h"

struct player_stall 
{
	struct entry_t
	{
		int type;
		size_t index;
		size_t count;
		size_t price;
		int crc;
		int expire_date;
	};

	item_list & _inv;
	abase::vector<entry_t, abase::fast_alloc<> > _goods_list;		//卖出列表
	char _market_name[PLAYER_MARKET_MAX_NAME_LEN];
	size_t _market_name_len;
	size_t sell_slot;
	size_t buy_slot;
public:
#pragma pack(1)
	struct trade_request
	{
		int trade_id;
		size_t money;
		size_t yinpiao;		//只在购买摊位上物品时有效
		size_t count;
		struct entry_t
		{
			int type;
			unsigned short index;
			unsigned short inv_index;		//仅摊位收购的时候有用
			unsigned int count;
		}list[];
	};
#pragma pack()
public:
	player_stall(item_list & inv):_inv(inv)
	{
		memset(_market_name,0,sizeof(_market_name));
		_market_name_len = 0;
		sell_slot = 0;
		buy_slot = 0;
	}
	
	void SetMarketName(const char name[PLAYER_MARKET_MAX_NAME_LEN])
	{
		size_t i;
		for(i =0; i <PLAYER_MARKET_MAX_NAME_LEN ; i +=2)
		{
			_market_name[i] = name[i];
			_market_name[i+1] = name[i+1];
			if(name[i] == 0 && name[i+1] == 0) 
			{
				i += 2;
				break;
			}
		}
		_market_name_len  = i;
	}

	inline const char *GetName()
	{
		return _market_name;
	}

	inline size_t GetNameLen()
	{
		return _market_name_len;
	}

	inline void SetSlot(size_t sslot, size_t bslot)
	{
		sell_slot = sslot;
		buy_slot = bslot;
	}
	
	inline size_t GetSellSlot(){ return sell_slot;}
	inline size_t GetBuySlot(){ return buy_slot;}

	inline void DecSellSlot(){ ASSERT(sell_slot); --sell_slot;}
	inline void DecBuySlot(){ ASSERT(buy_slot); --buy_slot;}
	
	void AddTradeGoods(size_t index,int type, size_t count, size_t price)
	{
		entry_t ent;
		ent.type = type;
		ent.index = index;
		ent.count = count;
		ent.price = price;
		ent.expire_date = _inv[index].expire_date;
		ent.crc = _inv[index].GetCRC();
		_goods_list.push_back(ent);
	}

	void AddOrderGoods(size_t index,int type, size_t count, size_t price)
	{
		entry_t ent;
		ent.type = type;
		ent.index = 0xFFFF;
		ent.count = count;
		ent.price = price;
		ent.crc = 0;
		ent.expire_date = 0;
		_goods_list.push_back(ent);
	}

	bool IsGoodsExist(int type)
	{
		for(size_t i=0; i<_goods_list.size(); i++)
		{
			if(_goods_list[i].type == type && _goods_list[i].index != 0xFFFF) return true;	
		}
		return false;
	}
};
#endif

