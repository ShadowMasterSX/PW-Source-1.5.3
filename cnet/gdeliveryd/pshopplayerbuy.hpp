#ifndef __GNET_PSHOPPLAYERBUY_HPP
#define __GNET_PSHOPPLAYERBUY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "dbpshopget.hrp"
#include "gamedbclient.hpp"
#include "gdeliveryserver.hpp"
#include "gproviderserver.hpp"
#include "pshopplayerbuy_re.hpp"

namespace GNET
{

class PShopPlayerBuy : public GNET::Protocol
{
	#include "pshopplayerbuy"

	void SyncGameServer(const PlayerInfo *pinfo, const GMailSyncData& data, int retcode)
	{
		GProviderServer::GetInstance()->DispatchProtocol(pinfo->gameid, GMailEndSync(0,retcode,roleid,data));
	}
	void SendErr(const PlayerInfo *pinfo, int retcode)
	{
		GDeliveryServer::GetInstance()->Send(pinfo->linksid, PShopPlayerBuy_Re(retcode, pinfo->localsid, master, item_id, item_pos, item_count));
	}
	bool QueryDB(const PlayerInfo & pinfo, const GMailSyncData & data) const
	{
		DBPShopPlayerBuyArg arg(roleid, master, item_id, item_pos, item_count, money_cost, yp_cost, data);
		DBPShopPlayerBuy *rpc = (DBPShopPlayerBuy *)Rpc::Call(RPC_DBPSHOPPLAYERBUY, arg);
		rpc->save_linksid = pinfo.linksid;
		rpc->save_localsid = pinfo.localsid;
		rpc->save_gsid = pinfo.gameid;
		return GameDBClient::GetInstance()->SendProtocol(rpc);
	}

	int CheckCondition(const PShopObj *obj) const
	{
		//买方包裹检查在gamed端已完成
		//这里只检查卖方店铺
		//1)是否有人在操作
		if(obj->IsBusy())
			return ERR_PLAYERSHOP_BUSY;

		//2)物品匹配检查
		const PShopItem *pItem = obj->GetItemSale(item_pos);
		if(!pItem)
			return ERR_PLAYERSHOP_ITEM_NOTFOUND;
		if((int)(pItem->item.id) != item_id || pItem->item.count < item_count)
			return ERR_PLAYERSHOP_ITEM_NOTMATCH;

		//3)客户端传过来的money_cost和yp_cost是否正确
		uint64_t total_cost = (uint64_t)money_cost + (uint64_t)yp_cost * (uint64_t)WANMEI_YINPIAO_PRICE;
		uint64_t item_value = (uint64_t)pItem->price * (uint64_t)item_count;
		if(total_cost != item_value)
			return ERR_PLAYERSHOP_VERIFY_FAILED;

		//4)金钱空间检查
		uint64_t total_money = obj->GetTotalMoney();
		uint64_t total_money_cap = obj->GetTotalMoneyCap();
		if((total_money_cap - total_money) < item_value)
			return ERR_PLAYERSHOP_VERIFY_FAILED;
		return ERR_SUCCESS;
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GMailSyncData data;
		try
		{
			Marshal::OctetsStream os(syncdata);
			os >> data;
		}
		catch(Marshal::Exception)
		{
			Log::log(LOG_ERR,"gdelivery::PShopPlayerBuy: unmarshal syncdata failed, roleid=%d", roleid);
			return;
		}

		Thread::RWLock::RDScoped lock(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if(!pinfo) return;
		
		if(!PShopMarket::GetInstance().IsLoadComplete())	
		{
			SendErr(pinfo, ERR_SHOPMARKET_NOT_INIT);
			SyncGameServer(pinfo, data, ERR_PLAYERSHOP_VERIFY_FAILED);
			return;
		}
		
		if(item_id <= 0 || item_pos < 0 || item_count <= 0 || (money_cost <= 0 && yp_cost <= 0) || pinfo->roleid == master)
		{
			SyncGameServer(pinfo, data, ERR_PLAYERSHOP_VERIFY_FAILED);
			return;
		}

		PShopObj *obj = PShopMarket::GetInstance().GetShop(master);
		if(!obj)
		{
			SendErr(pinfo, ERR_PLAYERSHOP_EXPIRED);
			SyncGameServer(pinfo, data, ERR_PLAYERSHOP_EXPIRED);
			return;
		}

		int retcode = CheckCondition(obj);
		if(retcode != ERR_SUCCESS)
		{
			SendErr(pinfo, retcode);
			SyncGameServer(pinfo, data, retcode);
			return;
		}
		if(!QueryDB(*pinfo, data))
		{
			SendErr(pinfo, ERR_FAILED);
			SyncGameServer(pinfo, data, ERR_FAILED);
			return;
		}

		obj->SetBusy(true);
	}
};

};

#endif
