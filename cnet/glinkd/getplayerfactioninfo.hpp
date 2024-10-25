
#ifndef __GNET_GETPLAYERFACTIONINFO_HPP
#define __GNET_GETPLAYERFACTIONINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gfactionclient.hpp"
namespace GNET
{

class GetPlayerFactionInfo : public GNET::Protocol
{
	#include "getplayerfactioninfo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if (!GLinkServer::ValidRole(sid,roleid))
			return;
		this->localsid=sid;
		GFactionClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
