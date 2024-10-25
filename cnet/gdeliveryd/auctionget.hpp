
#ifndef __GNET_AUCTIONGET_HPP
#define __GNET_AUCTIONGET_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "dbauctionget.hrp"
#include "mapuser.h"
#include "auctionget_re.hpp"
#include "auctionmarket.h"
#include "gdeliveryserver.hpp"
namespace GNET
{

class AuctionGet : public GNET::Protocol
{
	#include "auctionget"
	bool QueryDB( PlayerInfo& ui )
	{
		DBAuctionGet* rpc=(DBAuctionGet*) Rpc::Call( RPC_DBAUCTIONGET,DBAuctionGetArg(auctionid,0) );
		rpc->save_linksid=ui.linksid;
		rpc->save_localsid=ui.localsid;
		return GameDBClient::GetInstance()->SendProtocol( rpc );
	}
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("auctionget: receive. roleid=%d,localsid=%d,auctionid=%d\n",
				roleid,localsid,auctionid);
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if ( NULL!=pinfo )
		{
			AuctionGet_Re ag_re(ERR_SUCCESS,auctionid);
			if ( AuctionMarket::GetInstance().GetAuction(auctionid,ag_re.item) )
			{
				ag_re.localsid=pinfo->localsid;
				GDeliveryServer::GetInstance()->Send( pinfo->linksid,ag_re );
			}
			else
			{
				DEBUG_PRINT("auctionget: can not find auction in delivery. roleid=%d,localsid=%d,auctionid=%d\n",
					roleid,localsid,auctionid);
				
				if(AuctionMarket::GetInstance().IsMarketOpen()) {
					QueryDB( *pinfo );
				}
			}
		}
	}
};

};

#endif
