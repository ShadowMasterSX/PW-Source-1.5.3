
#ifndef __GNET_GETREWARDLIST_HPP
#define __GNET_GETREWARDLIST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class GetRewardList : public GNET::Protocol
{
	#include "getrewardlist"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if (!GLinkServer::ValidRole(sid, roleid))
			return; 
		localsid = sid;
		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
