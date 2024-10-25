
#ifndef __GNET_REMOTELOGOUT_HPP
#define __GNET_REMOTELOGOUT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class RemoteLogout : public GNET::Protocol
{
	#include "remotelogout"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(GDeliveryServer::GetInstance()->IsCentralDS()) return;
		
		UserInfo* userinfo = UserContainer::GetInstance().FindUser(userid);
		if(userinfo == NULL || (userinfo->status != _STATUS_REMOTE_HALFLOGIN && userinfo->status != _STATUS_REMOTE_LOGIN)) {
			Log::log(LOG_ERR, "Recv RemoteLogout, but user %d status %d invalid", userid, userinfo == NULL ? 0 : userinfo->status);
			return;
		} else {
			LOG_TRACE("Recv RemoteLogout userid %d status %d", userid, userinfo->status);
			STAT_MIN5("LogoutRemote", 1);
			UserContainer::GetInstance().UserLogout(userinfo, 0, true);
		}
	}
};

};

#endif
