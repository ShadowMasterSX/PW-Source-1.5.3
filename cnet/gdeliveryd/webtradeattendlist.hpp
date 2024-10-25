
#ifndef __GNET_WEBTRADEATTENDLIST_HPP
#define __GNET_WEBTRADEATTENDLIST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "webtrademarket.h"
#include "webtradeattendlist_re.hpp"

namespace GNET
{

class WebTradeAttendList : public GNET::Protocol
{
	#include "webtradeattendlist"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("webtradeattendlist: receive. roleid=%d,localsid=%d,getsell=%d,begin=%d \n", roleid,localsid,getsell,begin);
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( roleid );
		if ( NULL!=pinfo )
		{
			WebTradeAttendList_Re re;			
			re.getsell = getsell;
			re.localsid = localsid;
			WebTradeMarket::GetInstance().GetAttendWebTradeList(roleid,getsell,begin,re.items,re.end);
			
			GDeliveryServer::GetInstance()->Send( pinfo->linksid, re);	
		}
	}
};

};

#endif
