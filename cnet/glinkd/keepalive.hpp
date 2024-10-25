
#ifndef __GNET_KEEPALIVE_HPP
#define __GNET_KEEPALIVE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "glinkserver.hpp"
namespace GNET
{

class KeepAlive : public GNET::Protocol
{
	#include "keepalive"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if (manager==GLinkServer::GetInstance())
		{
			manager->Send(sid,this);
			GLinkServer* lsm = (GLinkServer*) manager;
			lsm->SetAliveTime(sid, _CLIENT_TTL);

			SessionInfo * sinfo = lsm->GetSessionInfo(sid);
			if (sinfo)
				sinfo->protostat.keepalive++;
		}
	}
};

};

#endif
