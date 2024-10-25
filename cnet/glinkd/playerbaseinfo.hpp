
#ifndef __GNET_PLAYERBASEINFO_HPP
#define __GNET_PLAYERBASEINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
namespace GNET
{

class PlayerBaseInfo : public GNET::Protocol
{
	#include "playerbaseinfo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if (!GLinkServer::ValidRole(sid,roleid))
		{
			GLinkServer::GetInstance()->SessionError(sid,ERR_INVALID_ACCOUNT,"Error userid or roleid.");
			return;
		}	
		if (playerlist.size() > 128) return;
		this->localsid=sid;
		GDeliveryClient::GetInstance()->SendProtocol(this);		

	}
};

};

#endif
