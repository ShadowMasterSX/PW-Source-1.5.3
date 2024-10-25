
#ifndef __GNET_AUCTIONGETITEM_HPP
#define __GNET_AUCTIONGETITEM_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "auctionmarket.h"
#include "auctiongetitem_re.hpp"
#include "mapuser.h"
#define MAX_IDSET_SIZE 8
namespace GNET
{

class AuctionGetItem : public GNET::Protocol
{
	#include "auctiongetitem"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("auctiongetitem: receive. roleid=%d,localsid=%d,idset.size=%d\n",
				roleid,localsid,ids.size());
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid );
		if ( NULL!=pinfo )
		{
			GAuctionDetail auction;
			AuctionGetItem_Re agi_re;
			agi_re.localsid = pinfo->localsid;
			for ( size_t i=0;i<ids.size() && i<MAX_IDSET_SIZE;++i )
			{
				if ( AuctionMarket::GetInstance().GetAuction(ids[i],auction) )
				{
					agi_re.ids.push_back( ids[i] );
					agi_re.items.push_back( auction.item  );
				}
			}
			GDeliveryServer::GetInstance()->Send( pinfo->linksid,agi_re );
		}
	}
};

};

#endif
