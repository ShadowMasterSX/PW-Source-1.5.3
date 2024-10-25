#ifndef __GNET_PSHOPCREATE_HPP
#define __GNET_PSHOPCREATE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gamedbclient.hpp"
#include "gproviderserver.hpp"
#include "gdeliveryserver.hpp"
#include "gmailendsync.hpp"
#include "pshopcreate_re.hpp"
#include "mapuser.h"
#include "pshopmarket.h"
#include "gmailsyncdata"

namespace GNET
{

class PShopCreate : public GNET::Protocol
{
	#include "pshopcreate"

	void SyncGameServer(const PlayerInfo *pinfo, const GMailSyncData& data, int retcode)
	{
		GProviderServer::GetInstance()->DispatchProtocol(pinfo->gameid, GMailEndSync(0,retcode,roleid,data));
	}
	void SendResult(const PlayerInfo *pinfo, int retcode)
	{
		GDeliveryServer::GetInstance()->Send(pinfo->linksid, PShopCreate_Re(retcode, pinfo->localsid));
	}
	bool QueryDB(const PlayerInfo & pinfo, const GMailSyncData & data)
	{
		DBPShopCreateArg arg(roleid, shoptype, item_id, item_pos, item_num, (int)time(NULL), data);
		DBPShopCreate * rpc = (DBPShopCreate *)Rpc::Call(RPC_DBPSHOPCREATE, arg);
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
			Log::log(LOG_ERR,"gdelivery::PShopCreate: unmarshal syncdata failed, roleid=%d", roleid);
			return;
		}

		Thread::RWLock::RDScoped lock(UserContainer::GetInstance().GetLocker());
		const PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if(!pinfo) return;
		
		if(!PShopMarket::GetInstance().IsLoadComplete())	
		{
			SendResult(pinfo, ERR_SHOPMARKET_NOT_INIT);
			SyncGameServer(pinfo, data, ERR_PLAYERSHOP_VERIFY_FAILED);
			return;
		}
		
		if(item_id <= 0 || item_pos < 0 || item_num <= 0 || shoptype < PSHOP_INDEX_MIN || shoptype >= PSHOP_INDEX_MAX)
		{
			SyncGameServer(pinfo, data, ERR_PLAYERSHOP_VERIFY_FAILED);
			return;
		}

		if(PShopMarket::GetInstance().GetShop(roleid))
		{
			//该玩家已经有店铺
			SendResult(pinfo, ERR_PLAYERSHOP_EXIST);
			SyncGameServer(pinfo, data, ERR_PLAYERSHOP_EXIST);
			return;
		}

		if(!QueryDB(*pinfo, data))
		{
			SendResult(pinfo, ERR_FAILED);
			SyncGameServer(pinfo, data, ERR_FAILED);
		}
	}
};

};

#endif
