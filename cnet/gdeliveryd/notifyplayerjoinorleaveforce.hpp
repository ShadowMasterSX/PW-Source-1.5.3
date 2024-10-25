
#ifndef __GNET_NOTIFYPLAYERJOINORLEAVEFORCE_HPP
#define __GNET_NOTIFYPLAYERJOINORLEAVEFORCE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "forcemanager.h"

namespace GNET
{

class NotifyPlayerJoinOrLeaveForce : public GNET::Protocol
{
	#include "notifyplayerjoinorleaveforce"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		ForceManager::GetInstance()->PlayerJoinOrLeave(force_id, is_join);
	}
};

};

#endif
