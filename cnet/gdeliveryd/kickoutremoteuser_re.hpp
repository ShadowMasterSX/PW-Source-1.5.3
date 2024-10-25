
#ifndef __GNET_KICKOUTREMOTEUSER_RE_HPP
#define __GNET_KICKOUTREMOTEUSER_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class KickoutRemoteUser_Re : public GNET::Protocol
{
	#include "kickoutremoteuser_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		LOG_TRACE("Recv KickoutRemoteUser_Re ret %d userid %d", retcode, userid);
		if(GDeliveryServer::GetInstance()->IsCentralDS()) return;
		
		if(retcode == ERR_SUCCESS) RemoteLoggingUsers::GetInstance().Pop(userid);
		UserContainer::GetInstance().ContinueLogin(userid, retcode == ERR_SUCCESS);
	}
};

};

#endif
