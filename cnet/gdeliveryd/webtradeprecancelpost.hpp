
#ifndef __GNET_WEBTRADEPRECANCELPOST_HPP
#define __GNET_WEBTRADEPRECANCELPOST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "webtrademarket.h"
#include "webtradeprecancelpost_re.hpp"

namespace GNET
{

class WebTradePreCancelPost : public GNET::Protocol
{
	#include "webtradeprecancelpost"

	void SendErr( int retcode,PlayerInfo& ui )
	{
		GDeliveryServer::GetInstance()->Send(
				ui.linksid,
				WebTradePreCancelPost_Re(retcode,sn,ui.localsid )
			);
	}

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("webtradeprecancelpost: receive. roleid=%d,localsid=%d,sn=%lld",roleid,localsid,sn);
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( roleid );
		if ( NULL == pinfo )
		{
			Log::log(LOG_ERR,"gdelivery::webtradeprecancelpost: role %d is not online", roleid);
			return;
		}

		int retcode = WebTradeMarket::GetInstance().TryPreCancelPost(roleid,sn,*pinfo);
		if(retcode != ERR_SUCCESS)
		{
			SendErr( retcode, *pinfo);	
		}
	}
};

};

#endif
