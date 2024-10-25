
#ifndef __GNET_FACTIONLISTONLINE_HPP
#define __GNET_FACTIONLISTONLINE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gfactionclient.hpp"

namespace GNET
{

class FactionListOnline : public GNET::Protocol
{
	#include "factionlistonline"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if (!GLinkServer::ValidRole(sid,roleid))
			return;
		this->localsid=sid;
		GFactionClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
