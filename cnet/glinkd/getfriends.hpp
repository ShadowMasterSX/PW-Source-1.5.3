
#ifndef __GNET_GETFRIENDS_HPP
#define __GNET_GETFRIENDS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class GetFriends : public GNET::Protocol
{
	#include "getfriends"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if (!GLinkServer::ValidRole(sid,roleid))
			return;
		localsid = sid;
		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
