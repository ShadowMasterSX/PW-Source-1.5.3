
#ifndef __GNET_DELFRIEND_HPP
#define __GNET_DELFRIEND_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class DelFriend : public GNET::Protocol
{
	#include "delfriend"

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
