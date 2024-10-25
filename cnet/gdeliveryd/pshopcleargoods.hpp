#ifndef __GNET_PSHOPCLEARGOODS_HPP
#define __GNET_PSHOPCLEARGOODS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "pshopmarket.h"
#include "gamedbclient.hpp"
#include "dbpshopcleargoods.hrp"

namespace GNET
{

class PShopClearGoods : public GNET::Protocol
{
	#include "pshopcleargoods"

	bool QueryDB(const PlayerInfo &pinfo) const
	{
		DBPShopClearGoodsArg arg(roleid);
		DBPShopClearGoods *rpc = (DBPShopClearGoods *)Rpc::Call(RPC_DBPSHOPCLEARGOODS, arg);
		rpc->save_linksid = pinfo.linksid;
		rpc->save_localsid = pinfo.localsid;
		return GameDBClient::GetInstance()->SendProtocol(rpc);
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Thread::RWLock::RDScoped lock(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if(!pinfo) return;
		
		if(!PShopMarket::GetInstance().IsLoadComplete())	
		{
			GDeliveryServer::GetInstance()->Send(pinfo->linksid, PShopClearGoods_Re(ERR_SHOPMARKET_NOT_INIT, pinfo->localsid));
			return;
		}
		

		//这里不查询店铺是否存在
		//因为店铺过期时也支持此操作

		if(!QueryDB(*pinfo))
		{
			//通知客户端清理失败
			GDeliveryServer::GetInstance()->Send(pinfo->linksid, PShopClearGoods_Re(ERR_FAILED, pinfo->localsid));
		}
	}
};

};

#endif
