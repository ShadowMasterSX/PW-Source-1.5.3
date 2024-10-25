
#ifndef __GNET_GETFACTIONBASEINFO_HPP
#define __GNET_GETFACTIONBASEINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gfactionclient.hpp"
namespace GNET
{

class GetFactionBaseInfo : public GNET::Protocol
{
	#include "getfactionbaseinfo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if (!GLinkServer::ValidRole(sid,roleid))
			return;
		if ( factionlist.size() > 128 ) return;
		this->localsid=sid;
		GFactionClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
