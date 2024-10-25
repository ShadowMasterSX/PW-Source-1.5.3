
#ifndef __GNET_DELETEROLE_HPP
#define __GNET_DELETEROLE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
namespace GNET
{

class DeleteRole : public GNET::Protocol
{
	#include "deleterole"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		this->localsid=sid;
		if ( GDeliveryClient::GetInstance()->SendProtocol(this) )
		{
			GLinkServer::GetInstance()->ChangeState(sid,&state_GDeleteRoleReceive);
		}
		else
			GLinkServer::GetInstance()->SessionError(sid,ERR_COMMUNICATION,"Server Network Error.");
	}
};

};

#endif
