
#ifndef __GNET_CASHLOCK_HPP
#define __GNET_CASHLOCK_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class CashLock : public GNET::Protocol
{
	#include "cashlock"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if (!GLinkServer::ValidRole(sid,userid))
			return;
		localsid = sid;
                GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
