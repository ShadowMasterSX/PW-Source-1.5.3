#ifndef __GNET_PSHOPLIST_HPP
#define __GNET_PSHOPLIST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "pshopmarket.h"
#include "pshoplist_re.hpp"
#include "gdeliveryserver.hpp"

namespace GNET
{

class PShopList : public GNET::Protocol
{
	#include "pshoplist"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Thread::RWLock::RDScoped lock(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if(!pinfo) return;
		
		if(!PShopMarket::GetInstance().IsLoadComplete())	
		{
			GDeliveryServer::GetInstance()->Send(pinfo->linksid, PShopList_Re(pinfo->localsid, PShopEntryVector()));
			return;
		}
		
		if(shoptype & ~PSHOP_MASK_ALL) return;//验证mask有效性

		//根据类型查找店铺时,支持单次查询多种类型店铺
		//此协议中的shoptype为店铺类型的掩码

		//将店铺类型mask转换为type
		int i = 0x01;
		int type = 0;
		int mask = shoptype;
		std::vector<char> typelist;
		while(mask)
		{
			if(mask & i)
			{
				mask &= ~i;
				typelist.push_back(type);
			}

			i <<= 1;
			++type;
		};

		//根据店铺类型查询
		PShopEntryVector shoplist;
		for(size_t i=0; i<typelist.size(); ++i)
		{
//			PShopEntryVector list;  
			PShopMarket::GetInstance().ListShops(typelist[i], shoplist/*list*/); //没必要push 2遍
//			for(size_t i=0; i<list.size(); ++i)
//				shoplist.push_back(list[i]);
		}

		GDeliveryServer::GetInstance()->Send(pinfo->linksid, PShopList_Re(pinfo->localsid, shoplist));
	}
};

};

#endif
