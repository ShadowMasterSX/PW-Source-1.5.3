#ifndef __GNET_PSHOPPLAYERSELL_HPP
#define __GNET_PSHOPPLAYERSELL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "dbpshopget.hrp"
#include "gamedbclient.hpp"
#include "gdeliveryserver.hpp"
#include "gproviderserver.hpp"
#include "pshopplayersell_re.hpp"

namespace GNET
{

class PShopPlayerSell : public GNET::Protocol
{
	#include "pshopplayersell"

	void SyncGameServer(const PlayerInfo *pinfo, const GMailSyncData& data, int retcode)
	{
		GProviderServer::GetInstance()->DispatchProtocol(pinfo->gameid, GMailEndSync(0,retcode,roleid,data));
	}
	void SendErr(const PlayerInfo *pinfo, int retcode)
	{
		GDeliveryServer::GetInstance()->Send(pinfo->linksid, PShopPlayerSell_Re(retcode,pinfo->localsid,master,item_id,item_pos,item_count));
	}
	bool QueryDB(const PlayerInfo & pinfo, const GMailSyncData & data) const
	{
		DBPShopPlayerSellArg arg(roleid, master, item_id, item_pos, item_count, inv_pos, data);
		DBPShopPlayerSell *rpc = (DBPShopPlayerSell *)Rpc::Call(RPC_DBPSHOPPLAYERSELL, arg);
		rpc->save_linksid = pinfo.linksid;
		rpc->save_localsid = pinfo.localsid;
		rpc->save_gsid = pinfo.gameid;
		return GameDBClient::GetInstance()->SendProtocol(rpc);
	}

	int CheckCondition(const PShopObj *obj) const
	{
		//包裹检查在gamed端已完成
		//这里只检查店铺
		//1)是否有人在操作
		if(obj->IsBusy())
			return ERR_PLAYERSHOP_BUSY;

		//2)物品匹配检查
		const PShopItem *pItem = obj->GetItemBuy(item_pos);
		if(!pItem)
			return ERR_PLAYERSHOP_ITEM_NOTFOUND;
		if((int)(pItem->item.id) != item_id || pItem->item.count < item_count || pItem->price != item_price)
			return ERR_PLAYERSHOP_ITEM_NOTMATCH;

		//3)店铺金钱检查
		uint64_t cost = (uint64_t)item_count * (uint64_t)pItem->price;
		uint64_t total_money = obj->GetTotalMoney();
		if(total_money < cost)
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
			Log::log(LOG_ERR,"gdelivery::PShopPlayerSell: unmarshal syncdata failed, roleid=%d", roleid);
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
		
		if(item_id <= 0 || item_pos < 0 || item_count <= 0 || inv_pos < 0 || pinfo->roleid == master)
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
