
#ifndef __GNET_SYSAUCTIONACCOUNT_HPP
#define __GNET_SYSAUCTIONACCOUNT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "sysauctionmanager.h"
#include "sysauctionaccount_re.hpp"
#include "gdeliveryserver.hpp"

namespace GNET
{

class SysAuctionAccount : public GNET::Protocol
{
	#include "sysauctionaccount"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("sysauctionaccount: receive. roleid=%d,localsid=%d", roleid, localsid);
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( roleid );
		if ( NULL!=pinfo )
		{
			SysAuctionAccount_Re re;
			re.localsid = localsid;
			re.retcode = SysAuctionManager::GetInstance().GetSysAuctionAccount(pinfo->userid, re.cash, re.bid_ids);
			GDeliveryServer::GetInstance()->Send(pinfo->linksid,re);
		}
	}
};

};

#endif
