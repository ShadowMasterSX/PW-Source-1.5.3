
#ifndef __GNET_AUCTIONEXITBID_HPP
#define __GNET_AUCTIONEXITBID_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "auctionmarket.h"
#include "auctionexitbid_re.hpp"
#include "gdeliveryserver.hpp"
#include "mapuser.h"

namespace GNET
{

class AuctionExitBid : public GNET::Protocol
{
	#include "auctionexitbid"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("auctionexitbid: receive. auctionid=%d,roleid=%d,localsid=%d\n",
				auctionid,roleid,localsid);
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( roleid );
		if ( NULL!=pinfo )
		{
			GDeliveryServer::GetInstance()->Send(
					pinfo->linksid,
					AuctionExitBid_Re(
						AuctionMarket::GetInstance().ExitAuction(roleid,auctionid) ? ERR_SUCCESS : ERR_GENERAL,
						pinfo->localsid,
						auctionid
					)
				);
		}
	}
};

};

#endif
