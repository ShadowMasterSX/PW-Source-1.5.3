#ifndef __GNET_PSHOPACTIVE_HPP
#define __GNET_PSHOPACTIVE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "pshopmarket.h"
#include "dbpshopactive.hrp"
#include "gamedbclient.hpp"
#include "gdeliveryserver.hpp"

namespace GNET
{

class PShopActive : public GNET::Protocol
{
	#include "pshopactive"

	void SyncGameServer(const PlayerInfo *pinfo, const GMailSyncData& data, int retcode)
	{
		GProviderServer::GetInstance()->DispatchProtocol(pinfo->gameid, GMailEndSync(0,retcode,roleid,data));
	}
	void SendResult(const PlayerInfo *pinfo, int retcode)
	{
		GDeliveryServer::GetInstance()->Send(pinfo->linksid, PShopActive_Re(retcode, pinfo->localsid));
	}
	bool QueryDB(const PlayerInfo & pinfo, const GMailSyncData &data) const
	{
		DBPShopActiveArg arg(roleid, item_id, item_pos, item_num, (int)time(NULL), data);
		DBPShopActive *rpc = (DBPShopActive *)Rpc::Call(RPC_DBPSHOPACTIVE, arg);
		rpc->save_linksid = pinfo.linksid;
		rpc->save_localsid = pinfo.localsid;
		rpc->save_gsid = pinfo.gameid;
		return GameDBClient::GetInstance()->SendProtocol(rpc);
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		//店铺处于正常和过期状态都可以进行此操作
		//正常状态则延长过期时间,
		//过期状态则将店铺激活

		GMailSyncData data;
		try
		{
			Marshal::OctetsStream os(syncdata);
			os >> data;
		}
		catch(Marshal::Exception)
		{
			Log::log(LOG_ERR,"gdelivery::PShopActive: unmarshal syncdata failed, roleid=%d", roleid);
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
		
		if(item_id <= 0 || item_pos < 0 || item_num <= 0)
		{
			SyncGameServer(pinfo, data, ERR_PLAYERSHOP_VERIFY_FAILED);
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
