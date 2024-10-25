
#ifndef __GNET_AUCTIONLISTUPDATE_HPP
#define __GNET_AUCTIONLISTUPDATE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class AuctionListUpdate : public GNET::Protocol
{
	#include "auctionlistupdate"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		//用于更新客户端中收藏的拍卖信息
		DEBUG_PRINT("auctionlistupdate: receive. roleid=%d,localsid=%d, ids_size=%d\n",
				roleid,localsid,ids.size());
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if ( NULL!=pinfo )
		{
			GAuctionDetail auction;
			AuctionListUpdate_Re alu_re( pinfo->localsid );
			for(size_t i=0; i<ids.size() && i<20; ++i)
			{
				if ( AuctionMarket::GetInstance().GetAuction(ids[i],auction) )
				{
					alu_re.items.push_back(auction.info);	
				}
				else
					alu_re.expired_ids.push_back(ids[i]);
			}
			GDeliveryServer::GetInstance()->Send( pinfo->linksid,alu_re );
		}
	}
};

};

#endif
