#ifndef __GNET_PSHOPPLAYERGET_HPP
#define __GNET_PSHOPPLAYERGET_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "pshopmarket.h"
#include "pshopplayerget_re.hpp"
#include "gdeliveryserver.hpp"

namespace GNET
{

class PShopPlayerGet : public GNET::Protocol
{
	#include "pshopplayerget"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Thread::RWLock::RDScoped lock(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if(!pinfo) return;
		
		if(!PShopMarket::GetInstance().IsLoadComplete())	
		{
			GDeliveryServer::GetInstance()->Send(pinfo->linksid, PShopPlayerGet_Re(ERR_SHOPMARKET_NOT_INIT, pinfo->localsid, PShopBase()));
			return;
		}

		const PShopObj *obj = PShopMarket::GetInstance().GetShop(master);
		if(obj)
		{
			GDeliveryServer::GetInstance()->Send(pinfo->linksid, PShopPlayerGet_Re(ERR_SUCCESS, pinfo->localsid, obj->GetShopBase()));
		}
	}
};

};

#endif
