
#ifndef __GNET_AUCTIONLIST_HPP
#define __GNET_AUCTIONLIST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "auctionlist_re.hpp"
#include "auctionmarket.h"
#include "mapuser.h"
namespace GNET
{

class AuctionList : public GNET::Protocol
{
	#include "auctionlist"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("auctionlist: receive. roleid=%d,localsid=%d,category=%d,item_id=%d,begin=%d,reverse=%d\n",
				roleid,localsid,category,item_id,begin,reverse);
		GDeliveryServer* dsm=GDeliveryServer::GetInstance();
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( roleid );
		if ( NULL!=pinfo )
		{
			AuctionList_Re al_re( pinfo->localsid,category );
			AuctionMarket::GetInstance().GetAuctionList( 
				AuctionMarket::find_param_t( category,item_id,begin,reverse==0 ),
				al_re.items,
				al_re.end
			);
			dsm->Send( pinfo->linksid,al_re );
		}
	}
};

};

#endif
