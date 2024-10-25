
#ifndef __GNET_UNDODELETEROLE_HPP
#define __GNET_UNDODELETEROLE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
namespace GNET
{

class UndoDeleteRole : public GNET::Protocol
{
	#include "undodeleterole"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		this->localsid=sid;
		if ( GDeliveryClient::GetInstance()->SendProtocol(this) )
		{
			GLinkServer::GetInstance()->ChangeState(sid,&state_GUndoDeleteRoleReceive);
		}
		else
			GLinkServer::GetInstance()->SessionError(sid,ERR_COMMUNICATION,"Server Network Error.");
	}
};

};

#endif
