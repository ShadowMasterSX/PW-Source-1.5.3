
#ifndef __GNET_KICKOUTUSER_HPP
#define __GNET_KICKOUTUSER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gauthclient.hpp"
#include "gdeliveryserver.hpp"
#include "gproviderserver.hpp"

#include "playerkickout.hpp"
#include <algorithm>

#include "trade.h"
#include "tradediscard.hpp"
#include "protocolexecutor.h"
#include "logoutroletask.h"
#include "mapforbid.h"
#include "mapuser.h"
namespace GNET
{
class KickoutUser : public GNET::Protocol
{
	#include "kickoutuser"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GDeliveryServer* gdsm=GDeliveryServer::GetInstance();
	
		UserInfo * pinfo = UserContainer::GetInstance().FindUser(userid);
		if (NULL == pinfo ) 
			return;
		this->localsid = pinfo->localsid;

                if(manager)
		{
			Log::formatlog("authd","kickoutuser:userid=%d:linksid=%d:gsid=%d:roleid=%d",
				userid, pinfo->linksid, pinfo->gameid, pinfo->roleid);
		}
		
		gdsm->Send(pinfo->linksid,this);
		
		UserContainer::GetInstance().UserLogout(pinfo, KICKOUT_LOCAL, true);
	}
};

};

#endif
