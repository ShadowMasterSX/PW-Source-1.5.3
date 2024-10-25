
#ifndef __GNET_WEBTRADELIST_HPP
#define __GNET_WEBTRADELIST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "webtrademarket.h"
#include "webtradelist_re.hpp"

namespace GNET
{

class WebTradeList : public GNET::Protocol
{
	#include "webtradelist"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("webtradelist: receive. roleid=%d,localsid=%d,category=%d,begin=%d,reverse=%d\n",
				roleid,localsid,category,begin,reverse);
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( roleid );
		if ( NULL!=pinfo )
		{
				WebTradeList_Re re;
				re.category = category;
				re.localsid = localsid;
				WebTradeMarket::GetInstance().GetWebTradeList(
					WebTradeMarket::find_param_t(category,begin,reverse==0,roleid),
					re.items,
					re.end
				);
				GDeliveryServer::GetInstance()->Send(pinfo->linksid,re);
		}
	}
};

};

#endif
