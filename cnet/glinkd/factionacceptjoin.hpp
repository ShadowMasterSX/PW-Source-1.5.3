
#ifndef __GNET_FACTIONACCEPTJOIN_HPP
#define __GNET_FACTIONACCEPTJOIN_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gfactionclient.hpp"
namespace GNET
{

class FactionAcceptJoin : public GNET::Protocol
{
	#include "factionacceptjoin"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if (!GLinkServer::ValidRole(sid,roleid))
		{
			GLinkServer::GetInstance()->SessionError(sid,ERR_INVALID_ACCOUNT,"Error userid or roleid.");
			return;
		}
		//send it to delivery
		this->localsid=sid;
		GFactionClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
