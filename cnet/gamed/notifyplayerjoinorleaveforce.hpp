
#ifndef __GNET_NOTIFYPLAYERJOINORLEAVEFORCE_HPP
#define __GNET_NOTIFYPLAYERJOINORLEAVEFORCE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class NotifyPlayerJoinOrLeaveForce : public GNET::Protocol
{
	#include "notifyplayerjoinorleaveforce"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
