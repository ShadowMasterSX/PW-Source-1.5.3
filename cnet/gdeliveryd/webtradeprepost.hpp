
#ifndef __GNET_WEBTRADEPREPOST_HPP
#define __GNET_WEBTRADEPREPOST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gmailsyncdata"
#include "gmailendsync.hpp"
#include "webtradeprepost_re.hpp"
#include "gproviderserver.hpp"
#include "gdeliveryserver.hpp"
#include "mapuser.h"
#include "webtrademarket.h"

namespace GNET
{

class WebTradePrePost : public GNET::Protocol
{
	#include "webtradeprepost"
	// this protocol is coming from gameserver
	void SyncGameServer( PlayerInfo& ui , const GMailSyncData& sync, int retcode)
	{
		GProviderServer::GetInstance()->DispatchProtocol(
				ui.gameid,
				GMailEndSync(0/*tid,must be 0*/,retcode,roleid,sync)
			);
	}
	
	void SendErr( int retcode,PlayerInfo& ui )
	{
		GDeliveryServer::GetInstance()->Send(
				ui.linksid,
				WebTradePrePost_Re(retcode,0,GWebTradeItem(),ui.localsid )
			);
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("webtradeprepost: receive. roleid=%d,localsid=%d,posttype=%d,money=%d,item_id=%d,item_pos=%d,item_num=%d,price=%d,sellperiod=%d,buyer_roleid=%d\n",
		roleid,localsid,posttype,money,item_id,item_pos,item_num,price,sellperiod,buyer_roleid);

		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( roleid );
		if ( NULL==pinfo )
		{
			Log::log(LOG_ERR,"gdelivery::webtradeprepost: role %d is not online", roleid);
			return;
		}
		UserInfo * userinfo = UserContainer::GetInstance().FindUser(pinfo->userid);
		if( NULL==userinfo)
		{
			Log::log(LOG_ERR,"gdelivery::webtradeprepost: user %d role %d is not online", pinfo->userid, roleid);
			return;
		}

		GMailSyncData sync;
		try
		{
			Marshal::OctetsStream os(syncdata);
			os >> sync;
		}catch(Marshal::Exception)
		{
			Log::log(LOG_ERR,"gdelivery::webtradeprepost: unmarshal syncdata failed, roleid=%d", roleid);
			return;
		}

		int retcode = WebTradeMarket::GetInstance().TryPrePost(*this, *pinfo, userinfo->ip, sync);
		if(retcode != ERR_SUCCESS)
		{
			SendErr( retcode,*pinfo );
			sync.inventory.items.clear();     
			SyncGameServer( *pinfo, sync, retcode );
		}
	}

};

};

#endif
