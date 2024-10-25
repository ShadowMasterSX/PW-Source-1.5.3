#ifndef __GNET_PSHOPDRAWITEM_HPP
#define __GNET_PSHOPDRAWITEM_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "pshopmarket.h"
#include "pshopdrawitem_re.hpp"
#include "dbpshopdrawitem.hrp"
#include "gamedbclient.hpp"
#include "gdeliveryserver.hpp"
#include "gproviderserver.hpp"

namespace GNET
{

class PShopDrawItem : public GNET::Protocol
{
	#include "pshopdrawitem"

	void SyncGameServer(const PlayerInfo *pinfo, const GMailSyncData& data, int retcode)
	{
		GProviderServer::GetInstance()->DispatchProtocol(pinfo->gameid, GMailEndSync(0,retcode,roleid,data));
	}
	void SendResult(const PlayerInfo *pinfo, int retcode)
	{
		GDeliveryServer::GetInstance()->Send(pinfo->linksid, PShopDrawItem_Re(retcode, pinfo->localsid, item_pos));
	}
	bool QueryDB(const PlayerInfo & pinfo, const GMailSyncData & data) const
	{
		DBPShopDrawItemArg arg(roleid, item_pos, data);
		DBPShopDrawItem *rpc = (DBPShopDrawItem *)Rpc::Call(RPC_DBPSHOPDRAWITEM, arg);
		rpc->save_linksid = pinfo.linksid;
		rpc->save_localsid = pinfo.localsid;
		rpc->save_gsid = pinfo.gameid;
		return GameDBClient::GetInstance()->SendProtocol(rpc);
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
			Log::log(LOG_ERR,"gdelivery::PShopDrawItem: unmarshal syncdata failed, roleid=%d", roleid);
			return;
		}
		Thread::RWLock::RDScoped lock(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if(!pinfo) return;
		
		if(!PShopMarket::GetInstance().IsLoadComplete())	
		{
			SendResult(pinfo, ERR_SHOPMARKET_NOT_INIT);
			SyncGameServer(pinfo, data, ERR_PLAYERSHOP_VERIFY_FAILED);
			return;
		}
		
		if(item_pos < 0)
		{
			SyncGameServer(pinfo, data, ERR_PLAYERSHOP_VERIFY_FAILED);
			return;
		}

		//店铺过期时也可以取物品
		//所以这里不查询PShopObj

		if(!QueryDB(*pinfo, data))
		{
			SendResult(pinfo, ERR_FAILED);
			SyncGameServer(pinfo, data, ERR_FAILED);
		}
	}
};

};

#endif
