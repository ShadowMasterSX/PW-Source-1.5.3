
#ifndef __GNET_AUCTIONCLOSE_HPP
#define __GNET_AUCTIONCLOSE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gamedbclient.hpp"
#include "gdeliveryserver.hpp"
#include "dbauctionclose.hrp"
#include "mapuser.h"
namespace GNET
{

class AuctionClose : public GNET::Protocol
{
	#include "auctionclose"
	void QueryDB( PlayerInfo& ui )
	{
		DBAuctionClose* rpc=(DBAuctionClose*) Rpc::Call( RPC_DBAUCTIONCLOSE,
				DBAuctionCloseArg(roleid,reason,auctionid));	
		rpc->save_linksid=ui.linksid;
		rpc->save_localsid=ui.localsid;
		GameDBClient::GetInstance()->SendProtocol( rpc );
	}
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("auctionclose: receive. roleid=%d,localsid=%d,auctionid=%d,reason=%d\n",
				roleid,localsid,auctionid,reason);
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( roleid );
		if ( NULL !=pinfo )
		{
			if(AuctionMarket::GetInstance().CanAuctionClose(auctionid))
				QueryDB( *pinfo );
		}
	}
};

};

#endif
