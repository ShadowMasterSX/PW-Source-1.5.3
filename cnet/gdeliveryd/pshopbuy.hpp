#ifndef __GNET_PSHOPBUY_HPP
#define __GNET_PSHOPBUY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "pshopitem"
#include "pshopmarket.h"
#include "dbpshopbuy.hrp"
#include "gamedbclient.hpp"
#include "gdeliveryserver.hpp"
#include "gproviderserver.hpp"

namespace GNET
{

class PShopBuy : public GNET::Protocol
{
	#include "pshopbuy"

	void SendErr(const PlayerInfo &pinfo, int retcode)
	{
			GDeliveryServer::GetInstance()->Send(pinfo.linksid, PShopBuy_Re(retcode,
															pinfo.localsid,
															item_id,
															item_pos,
															item_count,
															item_price));
	}

	bool QueryDB(const PlayerInfo & pinfo) const
	{
		DBPShopBuyArg arg(roleid, item_id, item_pos, item_count, item_price);
		DBPShopBuy *rpc = (DBPShopBuy *)Rpc::Call(RPC_DBPSHOPBUY, arg);
		rpc->save_linksid = pinfo.linksid;
		rpc->save_localsid = pinfo.localsid;
		return GameDBClient::GetInstance()->SendProtocol(rpc);
	}

	bool CheckCondition(const PShopObj *obj) const
	{
		if(obj->GetItemBuy(item_pos)) return false;//此栏位非空

		//计数收购栏花费
		//收购栏花费=本次收购花费+当前正在收购物品花费
		uint64_t need_money = (uint64_t)item_count * (uint64_t)item_price;
		const PShopItemVector &blist = obj->GetListBuy();
		for(size_t i=0; i<blist.size(); ++i)
			need_money += (uint64_t)blist[i].item.count * (uint64_t)blist[i].price;
		return need_money <= obj->GetTotalMoney();
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		//玩家收购

		Thread::RWLock::RDScoped lock(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if(!pinfo) return;
		
		if(!PShopMarket::GetInstance().IsLoadComplete())	
		{
			SendErr(*pinfo, ERR_SHOPMARKET_NOT_INIT);
			return;
		}
		
		if(item_id <= 0 || item_pos < 0 || item_count <= 0 || item_price <= 0) return;
		if(item_price <= 0 || item_price > (unsigned int)PSHOP_ITEM_PRICE_LIMIT) return;

		PShopObj * obj = PShopMarket::GetInstance().GetShop(roleid);
		if(!obj)
		{
			if(PShopMarket::GetInstance().GetFromTimeMap(roleid))
				SendErr(*pinfo, ERR_PLAYERSHOP_EXPIRED);
			return;
		}
		if(!CheckCondition(obj)) return;
		if(!QueryDB(*pinfo))
		{
			SendErr(*pinfo, ERR_FAILED);
		}
	}
};

};

#endif
