
#ifndef __GNET_PLAYERLOGOUT_HPP
#define __GNET_PLAYERLOGOUT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "statusannounce.hpp"
namespace GNET
{

class PlayerLogout : public GNET::Protocol
{
	#include "playerlogout"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer* lsm = GLinkServer::GetInstance();

		SessionInfo * sinfo = lsm->GetSessionInfo(localsid);
		if (!sinfo || sinfo->roleid!=roleid)
			return;
		sinfo->gsid   = 0;
		sinfo->roleid = 0;

		lsm->AccumulateSend(localsid,this);
		lsm->RoleLogout(roleid);
		//change state of linkserver
		if (result==_PLAYER_LOGOUT_FULL)
		{
			// todo check intention of readyclosetime
			lsm->SetReadyCloseTime(localsid, 30);
		}
	}
};

};

#endif
