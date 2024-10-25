#ifndef __GNET_PSHOPSETTYPE_HPP
#define __GNET_PSHOPSETTYPE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "pshopmarket.h"
#include "dbpshopsettype.hrp"

namespace GNET
{

class PShopSetType : public GNET::Protocol
{
	#include "pshopsettype"

	void SendErr(const PlayerInfo &pinfo, int retcode)
	{
		GDeliveryServer::GetInstance()->Send(pinfo.linksid, PShopSetType_Re(retcode, pinfo.localsid, -1));
	}
	bool QueryDB(const PlayerInfo & pinfo) const
	{
		DBPShopSetTypeArg arg(roleid, newtype);
		DBPShopSetType *rpc = (DBPShopSetType *)Rpc::Call(RPC_DBPSHOPSETTYPE, arg);
		rpc->save_linksid = pinfo.linksid;
		rpc->save_localsid = pinfo.localsid;
		return GameDBClient::GetInstance()->SendProtocol(rpc);
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Thread::RWLock::RDScoped lock(UserContainer::GetInstance().GetLocker());
		PlayerInfo *pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if(!pinfo) return;
		
		if(!PShopMarket::GetInstance().IsLoadComplete())	
		{
			SendErr(*pinfo, ERR_SHOPMARKET_NOT_INIT);
			return;
		}
		
		if(newtype < PSHOP_INDEX_MIN || newtype >= PSHOP_INDEX_MAX) return;

		PShopObj *obj = PShopMarket::GetInstance().GetShop(roleid);
		if(!obj && PShopMarket::GetInstance().GetFromTimeMap(roleid))
		{
			SendErr(*pinfo, ERR_PLAYERSHOP_EXPIRED);
			return;
		}

		if(!obj || obj->GetType() == newtype) return;
		if(!QueryDB(*pinfo))
		{
			//通知客户端修改类型失败
			SendErr(*pinfo, ERR_FAILED);
		}
	}
};

};

#endif
