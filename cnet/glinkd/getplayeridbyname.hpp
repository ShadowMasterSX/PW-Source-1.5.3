
#ifndef __GNET_GETPLAYERIDBYNAME_HPP
#define __GNET_GETPLAYERIDBYNAME_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
namespace GNET
{

class GetPlayerIDByName : public GNET::Protocol
{
	#include "getplayeridbyname"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if ( rolename.size()>MAX_NAME_SIZE)
			return;
		SessionInfo * sinfo = GLinkServer::GetInstance()->GetSessionInfo(sid);
		if (!sinfo || !sinfo->policy.Update(FINDNAME_POLICY))
			return;
		this->localsid = sid;
		GDeliveryClient::GetInstance()->SendProtocol(this);		
	}
};

};

#endif
