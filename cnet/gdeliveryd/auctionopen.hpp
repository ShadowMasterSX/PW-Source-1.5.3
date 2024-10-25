
#ifndef __GNET_AUCTIONOPEN_HPP
#define __GNET_AUCTIONOPEN_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "groleinventory"
#include "gmailsyncdata"
#include "gdeliveryserver.hpp"
#include "gamedbclient.hpp"
#include "gproviderserver.hpp"
#include "auctionopen_re.hpp"
#include "gmailendsync.hpp"
#include "auctionmarket.h"
#include "mapuser.h"

#define EIGHT_HOUR      28800
#define SIXTEEN_HOUR    57600
#define TWENTYFOUR_HOUR 86400
namespace GNET
{
class AuctionOpen : public GNET::Protocol
{
	#include "auctionopen"
	// this protocol is coming from gameserver
	void SyncGameServer( PlayerInfo& ui , const GMailSyncData& data, int retcode)
	{
#define GAuctionEndSync GMailEndSync
		GProviderServer::GetInstance()->DispatchProtocol(
				ui.gameid,
				GAuctionEndSync(0/*tid,must be 0*/,(short)retcode,roleid,data)
			);
#undef GAuctionEndSync		
	}

	bool QueryDB( PlayerInfo& ui,int new_auctionid,const GMailSyncData& data  )
	{
		int now = (int)time(NULL);
		DBAuctionOpen* rpc=(DBAuctionOpen*)Rpc::Call( 
				RPC_DBAUCTIONOPEN,
				DBAuctionOpenArg(
					roleid,
					new_auctionid,
					category,
					item_id,
					item_pos,
					item_num,
					baseprice,
					binprice,
					elapse_time,
					elapse_time+now+300, // get end time
					deposit,
					data
				)
			);
		rpc->save_linksid=ui.linksid;
		rpc->save_localsid=ui.localsid;
		rpc->save_gsid=ui.gameid;
		return GameDBClient::GetInstance()->SendProtocol( rpc );	
	}
	void SendErr( int retcode,PlayerInfo& ui )
	{
		GDeliveryServer::GetInstance()->Send(
				ui.linksid,
				AuctionOpen_Re(retcode,_AUCTIONID_INVALID,GAuctionItem(),ui.localsid )
			);
	}
	int CheckCondition()
	{
		AuctionMarket& market = AuctionMarket::GetInstance();
		if( !market.IsMarketOpen() ) return ERR_AS_MARKET_UNOPEN;
		if( !market.ValidCategory(item_id, category) ) return ERR_AS_MARKET_UNOPEN;
		if( market.GetAttendAuctionNum(roleid) >=MAX_ATTEND_AUCTION)
			return ERR_AS_ATTEND_OVF;	
		if(PostOffice::GetInstance().GetMailBoxSize(roleid) >= SYS_MAIL_LIMIT)
			return ERR_AS_MAILBOXFULL;
		return ERR_SUCCESS;
	}
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("auctionopen: receive. roleid=%d,localsid=%d,category=%d,item(pos:%d,count=%d)\n",
				roleid,localsid,category,item_pos,item_num);
		int retcode = CheckCondition();
		GMailSyncData data;
		try{
			Marshal::OctetsStream os(syncdata);
			os >> data;
		}catch(Marshal::Exception)
		{
			Log::log(LOG_ERR,"gdelivery::auctionopen: unmarshal syncdata failed, roleid=%d", roleid);
			return;
		}

		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( roleid );
		if ( NULL!=pinfo )
		{
			int new_auctionid = 0;
			if(retcode==ERR_SUCCESS)
			{
				new_auctionid = AuctionMarket::GetInstance().GetAuctionID();
				if(new_auctionid==_AUCTIONID_INVALID)  
					retcode = ERR_AS_ID_EXHAUSE;
			}
			if ( retcode==ERR_SUCCESS )
			{
				retcode = QueryDB( *pinfo,new_auctionid, data ) ? ERR_SUCCESS : ERR_AS_MARKET_UNOPEN;
			}
			if ( retcode!=ERR_SUCCESS )
			{
				SendErr( retcode,*pinfo );
				data.inventory.items.clear();     
				SyncGameServer( *pinfo, data, retcode );
			}
		}	
		else
			Log::log(LOG_ERR,"gdelivery::auctionopen: role %d is not online", roleid);

	}
};

};

#endif
