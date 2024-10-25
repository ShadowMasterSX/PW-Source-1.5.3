
#ifndef __GNET_STATUSANNOUNCE_HPP
#define __GNET_STATUSANNOUNCE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryserver.hpp"
#include "playeroffline.hpp"
#include "gauthclient.hpp"
#include <algorithm>
#include "userlogout.hrp"
#include "gauthclient.hpp"

#include "trade.h"
#include "tradediscard.hpp"
#include "conv_charset.h"
#include "logoutroletask.h"
#include "announceserverattribute.hpp"

#include "protocolexecutor.h"
#include "mapuser.h"
#include "maplinkserver.h"
namespace GNET
{

class StatusAnnounce : public GNET::Protocol
{
	#include "statusannounce"
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if (status!=_STATUS_OFFLINE) return;
		
		Thread::RWLock::WRScoped l(UserContainer::GetInstance().GetLocker());
		UserInfo * pinfo = UserContainer::GetInstance().FindUser(userid);
		
		//condition1: kickout user
		if (!pinfo || pinfo->localsid!=localsid) 
			return;
		if (pinfo->status==_STATUS_READYLOGOUT || pinfo->status==_STATUS_ONLINE)
		{
			DEBUG_PRINT("gdelivery:: statusannounce: user %d(roleid=%d) offline, onlineuser count is %d\n",
					userid,pinfo->roleid,UserContainer::GetInstance().Size());
			STAT_MIN5("LogoutNormal", 1);
		}
		else
		{
			DEBUG_PRINT("gdelivery::statusannounce: drop player, gsid=%d, userid=%d, roleid=%d",
					pinfo->gameid,userid, pinfo->roleid);
			STAT_MIN5("LogoutAbnormal", 1);
		}
		UserContainer::GetInstance().UserLogout(pinfo);
	}
};

};

#endif
