
#ifndef __GNET_WEBTRADEROLEPRECANCELPOST_HPP
#define __GNET_WEBTRADEROLEPRECANCELPOST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "webtrademarket.h"
#include "webtradeprecancelpost_re.hpp"

namespace GNET
{

class WebTradeRolePreCancelPost : public GNET::Protocol
{
	#include "webtraderoleprecancelpost"

	void SendErr( int retcode,UserInfo& ui )
	{
		GDeliveryServer::GetInstance()->Send(
				ui.linksid,
				WebTradePreCancelPost_Re(retcode,0,ui.localsid )
			);
	}
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("webtraderoleprecancelpost: receive. userid=%d,roleid=%d,localsid=%d",userid,roleid,localsid);
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		UserInfo * userinfo = UserContainer::GetInstance().FindUser(userid);
		if( NULL==userinfo || userinfo->logicuid!=LOGICUID(roleid) || !userinfo->rolelist.IsRoleExist(roleid)
				|| !userinfo->CheckCancelSellRole(roleid))
		{
			Log::log(LOG_ERR,"gdelivery::webtraderoleprecancelpost: user %d role %d invalid", userid, roleid);
			return;
		}

		int retcode = WebTradeMarket::GetInstance().TryRolePreCancelPost(roleid, *userinfo);
		if(retcode != ERR_SUCCESS)
		{
			SendErr( retcode, *userinfo);	
		}
	}
};

};

#endif
