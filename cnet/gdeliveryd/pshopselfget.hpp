#ifndef __GNET_PSHOPSELFGET_HPP
#define __GNET_PSHOPSELFGET_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryserver.hpp"
#include "pshopselfget_re.hpp"
#include "dbpshopget.hrp"
#include "pshopmarket.h"

namespace GNET
{

class PShopSelfGet : public GNET::Protocol
{
	#include "pshopselfget"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Thread::RWLock::RDScoped lock(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if(!pinfo) return;
		
		if(!PShopMarket::GetInstance().IsLoadComplete())	
		{
			GDeliveryServer::GetInstance()->Send(pinfo->linksid, PShopSelfGet_Re(ERR_SHOPMARKET_NOT_INIT, pinfo->localsid, PShopDetail()));
			return;
		}
		

		const PShopObj *obj = PShopMarket::GetInstance().GetShop(roleid);
		if(obj)
		{
			GDeliveryServer::GetInstance()->Send(pinfo->linksid, PShopSelfGet_Re(ERR_SUCCESS, pinfo->localsid, *obj));
		}
		else if(PShopMarket::GetInstance().GetFromTimeMap(roleid))
		{
			//店铺过期时店主获取
			DBPShopGet::QueryShop(roleid, pinfo->linksid, pinfo->localsid, 0, DBPShopGet::REASON_SELF_GET);
		}
		else
		{
			GDeliveryServer::GetInstance()->Send(pinfo->linksid, PShopSelfGet_Re(ERR_PLAYERSHOP_NOTFIND, pinfo->localsid, PShopDetail()));
		}
	}
};

};

#endif
