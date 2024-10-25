
#ifndef __GNET_SYSAUCTIONLIST_HPP
#define __GNET_SYSAUCTIONLIST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "sysauctionmanager.h"
#include "sysauctionlist_re.hpp"
#include "gdeliveryserver.hpp"

namespace GNET
{

class SysAuctionList : public GNET::Protocol
{
	#include "sysauctionlist"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("sysauctionlist: receive. roleid=%d,localsid=%d", roleid, localsid);
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( roleid );
		if ( NULL!=pinfo )
		{
			SysAuctionList_Re re;
			re.localsid = localsid;
			SysAuctionManager::GetInstance().GetSysAuctionList(re.items);
			GDeliveryServer::GetInstance()->Send(pinfo->linksid,re);
		}
	}
};

};

#endif
