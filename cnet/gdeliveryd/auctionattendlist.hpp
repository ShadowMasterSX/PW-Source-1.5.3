
#ifndef __GNET_AUCTIONATTENDLIST_HPP
#define __GNET_AUCTIONATTENDLIST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "auctionmarket.h"
#include "auctionattendlist_re.hpp"
#include "gdeliveryserver.hpp"
#include "mapuser.h"
#include <algorithm>
namespace GNET
{

class AuctionAttendList : public GNET::Protocol
{
	#include "auctionattendlist"
	void SendResult( int retcode,const AuctionAttendList_Re& aal_re,PlayerInfo& ui )
	{
		GDeliveryServer::GetInstance()->Send( ui.linksid, aal_re);		
	}
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("AuctionAttendList: receive. roleid=%d,localsid=%d\n", roleid,localsid);
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( roleid );
		if ( NULL!=pinfo )
		{
			AuctionAttendList_Re aal_re(localsid, 0);
			if ( AuctionMarket::GetInstance().GetAttendAuctionList( roleid,aal_re.items ) )
				SendResult( ERR_SUCCESS,aal_re,*pinfo );
			else
				SendResult( ERR_AS_MARKET_UNOPEN,aal_re,*pinfo );
		}
	}
};

};

#endif
