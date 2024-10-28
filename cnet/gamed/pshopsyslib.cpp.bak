#include "types.h"
#include "gmailsyncdata"
#include "obj_interface.h"
#include "pshopsyslib.h"
#include "libcommon.h"
#include "pshopcreate.hpp"
#include "pshopbuy.hpp"
#include "pshopsell.hpp"
#include "pshopcancelgoods.hpp"
#include "pshopsettype.hpp"
#include "pshopactive.hpp"
#include "pshopmanagefund.hpp"
#include "pshopdrawitem.hpp"
#include "pshopcleargoods.hpp"
#include "pshopselfget.hpp"
#include "pshopplayerbuy.hpp"
#include "pshopplayersell.hpp"
#include "gproviderclient.hpp"

namespace GNET
{

#define GDELIVERY_SERVER_ID	0
#define PSHOP_TYPE_MIN		0
#define PSHOP_TYPE_MAX		8

#define CASE_PROTO_HANDLE(_proto_name_)\
	case _proto_name_::PROTOCOL_TYPE:\
	{\
		_proto_name_ proto;\
		proto.unmarshal( os );\
		if ( proto.GetType()!=_proto_name_::PROTOCOL_TYPE || !proto.SizePolicy(os.size()) )\
			return false;\
		return Handle_##_proto_name_( proto,player);\
	}

static bool QuerySyncData(Octets &syncdata, object_interface &player)
{
	GMailSyncData data;
	if(!GetSyncData(data, player))
		return false;

	Marshal::OctetsStream os;
	os << data;
	syncdata.swap(os);
	return true;
}

/*
 * 函数功能: 金钱转换为银票
 * @param: money: 需要兑换的金钱数
 * @param: yinpiao: 兑换得到的银票
 * @param: remains: 剩余金钱数
 */
static void MoneyToYinPiao(uint64_t money, unsigned int &yinpiao, unsigned int &remains)
{
	yinpiao = 0;
	remains = 0;
	while(money >= WANMEI_YINPIAO_PRICE)
	{
		yinpiao++;
		money -= WANMEI_YINPIAO_PRICE;
	}
	remains = money;
}

bool Handle_PShopCreate(PShopCreate &proto, object_interface &player)
{
	if(proto.roleid != player.GetSelfID().id) return false;
	if(proto.shoptype < PSHOP_TYPE_MIN || proto.shoptype >= PSHOP_TYPE_MAX) return false;
	if(proto.item_id <= 0 || proto.item_pos < 0 || proto.item_num <= 0) return false;
	if(proto.item_id != PSHOP_CONSUME_ITEM_1 && proto.item_id != PSHOP_CONSUME_ITEM_2 && proto.item_id != PSHOP_CONSUME_ITEM_3) return false;
	if(!player.CheckItem(proto.item_pos, proto.item_id, proto.item_num)) return false;
	if(!QuerySyncData(proto.syncdata, player)) return false;
	if(player.TradeLockPlayer(0,DBMASK_PUT_SYNC_TIMEOUT) == 0)
	{
		if(GProviderClient::GetInstance()->DispatchProtocol(GDELIVERY_SERVER_ID, proto))
			return true;
		player.TradeUnLockPlayer();
	}
	return false;
}

bool Handle_PShopBuy(PShopBuy &proto, object_interface &player)
{
	if(proto.roleid != player.GetSelfID().id) return false;
	if(proto.item_id <= 0 || proto.item_pos < 0 || proto.item_count <=0) return false;
	if(proto.item_price <= 0 || proto.item_price > (unsigned int)PSHOP_ITEM_PRICE_LIMIT) return false;
	return GProviderClient::GetInstance()->DispatchProtocol(GDELIVERY_SERVER_ID, proto);
}

bool Handle_PShopSell(PShopSell &proto, object_interface &player)
{
	if(proto.roleid != player.GetSelfID().id) return false;
	if(proto.inv_pos < 0 || proto.item_id <= 0 || proto.item_pos < 0 || proto.item_count <=0) return false;
	if(proto.item_price <= 0 || proto.item_price > (unsigned int)PSHOP_ITEM_PRICE_LIMIT) return false;
	if(!player.CheckItem(proto.inv_pos, proto.item_id, proto.item_count)) return false;
	if(!QuerySyncData(proto.syncdata, player)) return false;
	if(player.TradeLockPlayer(0,DBMASK_PUT_SYNC_TIMEOUT) == 0)
	{
		if(GProviderClient::GetInstance()->DispatchProtocol(GDELIVERY_SERVER_ID, proto))
			return true;
		player.TradeUnLockPlayer();
	}
	return false;
}

bool Handle_PShopCancelGoods(PShopCancelGoods &proto, object_interface &player)
{
	if(proto.roleid != player.GetSelfID().id) return false;
	if(proto.pos < 0) return false;
	if(proto.canceltype != 0 && proto.canceltype != 1) return false;
	return GProviderClient::GetInstance()->DispatchProtocol(GDELIVERY_SERVER_ID, proto);
}

bool Handle_PShopSetType(PShopSetType &proto, object_interface &player)
{
	if(proto.roleid != player.GetSelfID().id) return false;
	if(proto.newtype < PSHOP_TYPE_MIN || proto.newtype >= PSHOP_TYPE_MAX) return false;
	return GProviderClient::GetInstance()->DispatchProtocol(GDELIVERY_SERVER_ID, proto);
}

bool Handle_PShopActive(PShopActive &proto, object_interface &player)
{
	if(proto.roleid != player.GetSelfID().id) return false;
	if(proto.item_id <= 0 || proto.item_pos < 0 || proto.item_num <= 0) return false;
	if(proto.item_id != PSHOP_CONSUME_ITEM_1 && proto.item_id != PSHOP_CONSUME_ITEM_2 && proto.item_id != PSHOP_CONSUME_ITEM_3) return false;
	if(!player.CheckItem(proto.item_pos, proto.item_id, proto.item_num)) return false;
	if(!QuerySyncData(proto.syncdata,player)) return false;
	if(player.TradeLockPlayer(0,DBMASK_PUT_SYNC_TIMEOUT) == 0)
	{
		if(GProviderClient::GetInstance()->DispatchProtocol(GDELIVERY_SERVER_ID, proto))
			return true;
		player.TradeUnLockPlayer();
	}
	return false;
}

bool Handle_PShopManageFund(PShopManageFund &proto, object_interface &player)
{
	if(proto.roleid != player.GetSelfID().id) return false;
	if(proto.optype != 0 && proto.optype != 1) return false;
	if(proto.money < 0 || proto.yinpiao < 0) return false;
	if(proto.money == 0 && proto.yinpiao == 0) return false;
	if(proto.optype == 0)//存钱
	{
		if(player.GetMoney() < proto.money) return false;
		if(proto.yinpiao > 0 && !player.CheckItem(WANMEI_YINPIAO_ID, proto.yinpiao)) return false;
	}
	else if(proto.optype == 1)//取钱
	{
		unsigned int money_gain = proto.money;
		unsigned int yp_gain = proto.yinpiao;
		if(proto.money >= WANMEI_YINPIAO_PRICE)
		{
			unsigned int yp=0;
			unsigned int remain=0;
			MoneyToYinPiao(proto.money, yp, remain);
			money_gain = remain;
			yp_gain += yp;
		}

		unsigned int need_slot = 0;
		while(yp_gain > 0)
		{
			++need_slot;
			if(yp_gain <= WANMEI_YINPIAO_PILELIMIT) break;
			yp_gain -= WANMEI_YINPIAO_PILELIMIT;
		};
		if(player.GetEmptySlotSize() < need_slot) return false;//包裹满(未考虑银票堆叠存放)
		if((MAX_CASH_IN_POCKET - player.GetMoney()) < money_gain) return false;//金钱满
	}

	if(!QuerySyncData(proto.syncdata,player)) return false;
	if(player.TradeLockPlayer(0,DBMASK_PUT_SYNC_TIMEOUT) == 0)
	{
		if(GProviderClient::GetInstance()->DispatchProtocol(GDELIVERY_SERVER_ID, proto))
			return true;
		player.TradeUnLockPlayer();
	}
	return false;
}

bool Handle_PShopDrawItem(PShopDrawItem &proto, object_interface &player)
{
	if(proto.roleid != player.GetSelfID().id) return false;
	if(proto.item_pos < 0) return false;
	if(player.IsInventoryFull()) return false;
	if(!QuerySyncData(proto.syncdata,player)) return false;
	if(player.TradeLockPlayer(0,DBMASK_PUT_SYNC_TIMEOUT) == 0)
	{
		if(GProviderClient::GetInstance()->DispatchProtocol(GDELIVERY_SERVER_ID, proto))
			return true;
		player.TradeUnLockPlayer();
	}
	return false;
}

bool Handle_PShopClearGoods(PShopClearGoods &proto, object_interface &player)
{
	if(proto.roleid != player.GetSelfID().id) return false;
	return GProviderClient::GetInstance()->DispatchProtocol(GDELIVERY_SERVER_ID, proto);
}

bool Handle_PShopSelfGet(PShopSelfGet &proto, object_interface &player)
{
	if(proto.roleid != player.GetSelfID().id) return false;
	return GProviderClient::GetInstance()->DispatchProtocol(GDELIVERY_SERVER_ID, proto);
}

bool Handle_PShopPlayerBuy(PShopPlayerBuy &proto, object_interface &player)
{
	if(proto.roleid == proto.master) return false;
	if(proto.roleid != player.GetSelfID().id) return false;
	if(proto.item_id <= 0 || proto.item_pos < 0 || proto.item_count <= 0) return false;

	//检查金钱和银票是否够
	if(player.GetMoney() < (unsigned int)proto.money_cost) return false;
	if(proto.yp_cost > 0 && !player.CheckItem(WANMEI_YINPIAO_ID, proto.yp_cost)) return false;

	if(player.IsInventoryFull()) return false;//包裹满
	if(!QuerySyncData(proto.syncdata,player)) return false;
	if(player.TradeLockPlayer(0,DBMASK_PUT_SYNC_TIMEOUT) == 0)
	{
		if(GProviderClient::GetInstance()->DispatchProtocol(GDELIVERY_SERVER_ID, proto))
			return true;
		player.TradeUnLockPlayer();
	}
	return false;
}

bool Handle_PShopPlayerSell(PShopPlayerSell &proto, object_interface &player)
{
	if(proto.roleid == proto.master) return false;
	if(proto.roleid != player.GetSelfID().id) return false;
	if(proto.item_id <= 0 || proto.item_pos < 0 || proto.item_count <= 0 || proto.inv_pos < 0) return false;
	if(proto.item_price <= 0 || proto.item_price > (unsigned int)PSHOP_ITEM_PRICE_LIMIT) return false;

	//检查包裹
	uint64_t money_gain = (uint64_t)proto.item_count * (uint64_t)proto.item_price;
	if((MAX_CASH_IN_POCKET - player.GetMoney()) < money_gain)
	{
		unsigned int yp=0;
		unsigned int remain=0;
		MoneyToYinPiao(money_gain, yp, remain);
		if((MAX_CASH_IN_POCKET - player.GetMoney()) < remain) return false;

		unsigned int need_slot = 0;
		while(yp >= WANMEI_YINPIAO_PILELIMIT)
		{
			++need_slot;
			yp -= WANMEI_YINPIAO_PILELIMIT;
		};

		if(yp > 0) need_slot += 1;
		if(player.GetEmptySlotSize() < need_slot) return false;//包裹满,未考虑银票堆叠存放
	}

	if(!player.CheckItem(proto.inv_pos, proto.item_id, proto.item_count)) return false;
	if(!QuerySyncData(proto.syncdata,player)) return false;
	if(player.TradeLockPlayer(0,DBMASK_PUT_SYNC_TIMEOUT) == 0)
	{
		if(GProviderClient::GetInstance()->DispatchProtocol(GDELIVERY_SERVER_ID, proto))
			return true;
		player.TradeUnLockPlayer();
	}
	return false;
}

bool ForwardPShopSysOP(unsigned int type,const void* pParams,size_t param_len,object_interface player)
{
	try
	{
		Marshal::OctetsStream os(Octets(pParams,param_len));
		switch(type)
		{
			CASE_PROTO_HANDLE(PShopCreate)
			CASE_PROTO_HANDLE(PShopBuy)
			CASE_PROTO_HANDLE(PShopSell)
			CASE_PROTO_HANDLE(PShopCancelGoods)
			CASE_PROTO_HANDLE(PShopSetType)
			CASE_PROTO_HANDLE(PShopActive)
			CASE_PROTO_HANDLE(PShopManageFund)
			CASE_PROTO_HANDLE(PShopDrawItem)
			CASE_PROTO_HANDLE(PShopClearGoods)
			CASE_PROTO_HANDLE(PShopSelfGet)
			CASE_PROTO_HANDLE(PShopPlayerBuy)
			CASE_PROTO_HANDLE(PShopPlayerSell)
			default:
				return false;		
		}
	}
	catch(Marshal::Exception)
	{
		return false;
	}
}

};
