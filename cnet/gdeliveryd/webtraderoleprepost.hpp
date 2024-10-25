
#ifndef __GNET_WEBTRADEROLEPREPOST_HPP
#define __GNET_WEBTRADEROLEPREPOST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "webtradeprepost_re.hpp"
#include "gdeliveryserver.hpp"
#include "mapuser.h"
#include "webtrademarket.h"

namespace GNET
{

class WebTradeRolePrePost : public GNET::Protocol
{
	#include "webtraderoleprepost"

	void SendErr( int retcode,UserInfo& ui )
	{
		GDeliveryServer::GetInstance()->Send(
				ui.linksid,
				WebTradePrePost_Re(retcode,0,GWebTradeItem(),ui.localsid )
			);
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("webtraderoleprepost: receive. userid=%d,roleid=%d,localsid=%d,price=%d,sellperiod=%d,buyer_roleid=%d\n",
		userid,roleid,localsid,price,sellperiod,buyer_roleid);
		
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		UserInfo * userinfo = UserContainer::GetInstance().FindUser(userid);
		if( NULL==userinfo || userinfo->logicuid!=LOGICUID(roleid) || !userinfo->rolelist.IsRoleExist(roleid) 
				|| !userinfo->CheckSellRole(roleid))
		{
			Log::log(LOG_ERR,"gdelivery::webtraderoleprepost: user %d role %d invalid", userid, roleid);
			return;
		}
		//role可否交易在gamedbd检查
		int retcode = WebTradeMarket::GetInstance().TryRolePrePost(*this, *userinfo);
		if(retcode != ERR_SUCCESS)
		{
			SendErr( retcode,*userinfo);
		}
	}
};

};

#endif
