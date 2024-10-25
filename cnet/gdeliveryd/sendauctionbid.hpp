
#ifndef __GNET_SENDAUCTIONBID_HPP
#define __GNET_SENDAUCTIONBID_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class SendAuctionBid : public GNET::Protocol
{
	#include "sendauctionbid"

	bool QueryDB( PlayerInfo& ui ) 
	{
		DBAuctionBid* rpc=(DBAuctionBid*) Rpc::Call(
				RPC_DBAUCTIONBID,
				DBAuctionBidArg( roleid,auctionid,bidprice,bin,syncdata )
			);	
		rpc->save_linksid=ui.linksid;
		rpc->save_localsid=ui.localsid;
		rpc->save_gsid=ui.gameid;
		return GameDBClient::GetInstance()->SendProtocol( rpc );	
	}
	void SendErr( int retcode,PlayerInfo& ui, unsigned int newprice)
	{
		GDeliveryServer::GetInstance()->Send(ui.linksid, AuctionBid_Re(retcode,newprice,
				auctionid,GAuctionItem(),ui.localsid));
		syncdata.inventory.items.clear();
		GProviderServer::GetInstance()->DispatchProtocol(ui.gameid, GMailEndSync(0,retcode,roleid,syncdata));	
	}
	int CheckCondition(unsigned int aid, unsigned int& newprice)
	{
		AuctionMarket& market = AuctionMarket::GetInstance();
		int ret = market.ValidPrice(aid, bidprice, bin, newprice);
		if(ret)
			return ret;
		if (market.GetAttendAuctionNum(roleid) >= MAX_ATTEND_AUCTION )
			return ERR_AS_ATTEND_OVF;	
		if( PostOffice::GetInstance().GetMailBoxSize(roleid) >= SYS_MAIL_LIMIT)
			return ERR_AS_MAILBOXFULL;	
		return ERR_SUCCESS;
	}
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("auctionbid: receive. roleid=%d,localsid=%d,auctionid=%d,bidprice=%d,bin=%d,own_money=%d\n",
				roleid,localsid,auctionid,bidprice,bin,syncdata.inventory.money);

		unsigned int newprice; 
		int retcode = CheckCondition(auctionid, newprice);

		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( (roleid) );
		if ( NULL!=pinfo)
		{
			if ( retcode==ERR_SUCCESS )
				QueryDB( *pinfo );
			else
				SendErr( retcode,*pinfo,newprice );
		}		
		else
			Log::log(LOG_ERR,"gdelivery::auctionbid: role %d is not online", roleid);
	}
};

};

#endif
