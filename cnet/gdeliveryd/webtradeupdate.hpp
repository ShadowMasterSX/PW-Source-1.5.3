
#ifndef __GNET_WEBTRADEUPDATE_HPP
#define __GNET_WEBTRADEUPDATE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "webtradeupdate_re.hpp"

namespace GNET
{

class WebTradeUpdate : public GNET::Protocol
{
	#include "webtradeupdate"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("webtradeupdate: receive. roleid=%d,localsid=%d,sn=%lld\n",
				roleid,localsid,sn);
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid );
		if ( NULL!=pinfo )
		{
			WebTradeUpdate_Re re;
			re.sn = sn;
			re.localsid = localsid; 
			GWebTradeDetail detail;
			if ( WebTradeMarket::GetInstance().GetWebTrade(sn,detail) )
			{
				re.retcode = ERR_SUCCESS;
				re.item = detail.info;
			}
			else
				re.retcode = ERR_WT_ENTRY_NOT_FOUND;
			GDeliveryServer::GetInstance()->Send( pinfo->linksid,re );
		}
	}
};

};

#endif
