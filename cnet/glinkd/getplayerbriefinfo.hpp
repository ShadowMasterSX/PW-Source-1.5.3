
#ifndef __GNET_GETPLAYERBRIEFINFO_HPP
#define __GNET_GETPLAYERBRIEFINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
namespace GNET
{

class GetPlayerBriefInfo : public GNET::Protocol
{
	#include "getplayerbriefinfo"

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
