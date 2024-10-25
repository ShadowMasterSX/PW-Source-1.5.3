
#ifndef __GNET_CREATEROLE_HPP
#define __GNET_CREATEROLE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
#include "groleinventory"
#include "roleinfo"
namespace GNET
{

class CreateRole : public GNET::Protocol
{
	#include "createrole"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if (!GLinkServer::GetInstance()->ValidUser(sid,userid)) return;
		this->localsid=sid;
		if ( GDeliveryClient::GetInstance()->SendProtocol(this) )
		{
			GLinkServer::GetInstance()->ChangeState(sid,&state_GCreateRoleReceive);
		}
		else
			GLinkServer::GetInstance()->SessionError(sid,ERR_COMMUNICATION,"Server Network Error.");
	}
};

};

#endif
