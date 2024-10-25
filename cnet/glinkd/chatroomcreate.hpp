
#ifndef __GNET_CHATROOMCREATE_HPP
#define __GNET_CHATROOMCREATE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class ChatRoomCreate : public GNET::Protocol
{
	#include "chatroomcreate"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		localsid = sid;
		if (!GLinkServer::ValidRole(sid,roleid))
		{
			GLinkServer::GetInstance()->SessionError(sid,ERR_INVALID_ACCOUNT,"Error userid or roleid.");
			return;
		}	
		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
