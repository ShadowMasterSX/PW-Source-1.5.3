
#ifndef __GNET_SYSAUCTIONBID_HPP
#define __GNET_SYSAUCTIONBID_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "sysauctionmanager.h"
#include "sysauctionbid_re.hpp"
#include "gdeliveryserver.hpp"

namespace GNET
{

class SysAuctionBid : public GNET::Protocol
{
	#include "sysauctionbid"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("sysauctionbid: receive. roleid=%d,localsid=%d,sa_id=%u,bid_price=%u", roleid, localsid, sa_id, bid_price);
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( roleid );
		if ( NULL==pinfo ) return;

		SysAuctionBid_Re re;
		re.localsid = localsid;
		re.retcode = SysAuctionManager::GetInstance().TryBid(pinfo->userid, roleid, sa_id, bid_price, re.cash, re.info);
		GDeliveryServer::GetInstance()->Send(pinfo->linksid,re);
	}
};

};

#endif
