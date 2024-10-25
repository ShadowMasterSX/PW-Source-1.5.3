#ifndef __GNET_PSHOPSELL_HPP
#define __GNET_PSHOPSELL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "pshopmarket.h"
#include "dbpshopsell.hrp"
#include "gamedbclient.hpp"
#include "gdeliveryserver.hpp"
#include "gproviderserver.hpp"

namespace GNET
{

class PShopSell : public GNET::Protocol
{
	#include "pshopsell"

	void SyncGameServer(const PlayerInfo *pinfo, const GMailSyncData& data, int retcode)
	{
		GProviderServer::GetInstance()->DispatchProtocol(pinfo->gameid, GMailEndSync(0,retcode,roleid,data));
	}
	void SendErr(const PlayerInfo &pinfo, int retcode)
	{
		GDeliveryServer::GetInstance()->Send(pinfo.linksid, PShopSell_Re(retcode,
																		pinfo.localsid,
																		item_id,
																		item_pos,
																		item_count,
																		item_price,
																		inv_pos));
	}
	bool QueryDB(const PlayerInfo & pinfo, const GMailSyncData & data) const
	{
		DBPShopSellArg arg(roleid, item_id, item_pos, item_count, item_price, inv_pos, data);
		DBPShopSell *rpc = (DBPShopSell *)Rpc::Call(RPC_DBPSHOPSELL, arg);
		rpc->save_linksid = pinfo.linksid;
		rpc->save_localsid = pinfo.localsid;
		rpc->save_gsid = pinfo.gameid;
		return GameDBClient::GetInstance()->SendProtocol(rpc);
	}
	bool CheckCondition(const PShopObj *obj) const
	{
		if(obj->GetItemSale(item_pos)) return false;//此栏位非空

		//计数出售栏物品价值
		uint64_t total_item_value = 0;
		const PShopItemVector &slist = obj->GetListSale();
		for(size_t i=0; i<slist.size(); ++i)
			total_item_value += (uint64_t)slist[i].item.count * (uint64_t)slist[i].price;
		total_item_value += ((uint64_t)item_count * (uint64_t)item_price);
		return obj->GetTotalMoneyCap() - obj->GetTotalMoney() >= total_item_value;
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
			Log::log(LOG_ERR,"gdelivery::PShopSell: unmarshal syncdata failed, roleid=%d", roleid);
			return;
		}

		Thread::RWLock::RDScoped lock(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if(!pinfo) return;
		
		if(!PShopMarket::GetInstance().IsLoadComplete())	
		{
			SendErr(*pinfo, ERR_SHOPMARKET_NOT_INIT);
			SyncGameServer(pinfo, data, ERR_PLAYERSHOP_VERIFY_FAILED);
			return;
		}
		
		if(inv_pos < 0 || item_id <= 0 || item_pos < 0 || item_count <= 0 || item_price <= 0 || item_price > (unsigned int)PSHOP_ITEM_PRICE_LIMIT)
		{
			SyncGameServer(pinfo, data, ERR_PLAYERSHOP_VERIFY_FAILED);
			return;
		}

		PShopObj *obj = PShopMarket::GetInstance().GetShop(roleid);
		if(!obj)
		{
			SyncGameServer(pinfo, data, ERR_PLAYERSHOP_NOTFIND);
			if(PShopMarket::GetInstance().GetFromTimeMap(roleid))
			{
				SendErr(*pinfo, ERR_PLAYERSHOP_EXPIRED);
			}
			return;
		}

		if(!CheckCondition(obj))
		{
			SyncGameServer(pinfo, data, ERR_PLAYERSHOP_VERIFY_FAILED);
			return;
		}

		if(!QueryDB(*pinfo, data))
		{
			SyncGameServer(pinfo, data, ERR_FAILED);
			SendErr(*pinfo, ERR_FAILED);
		}
	}
};

};

#endif
