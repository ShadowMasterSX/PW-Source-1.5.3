
#ifndef __GNET_NOTIFYFACTIONFORTRESSSTATE_HPP
#define __GNET_NOTIFYFACTIONFORTRESSSTATE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "factionfortressmanager.h"

namespace GNET
{

class NotifyFactionFortressState : public GNET::Protocol
{
	#include "notifyfactionfortressstate"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("notifyfactionfortressstate: receive. factionid=%d,state=%d",factionid,state);
		FactionFortressMan::GetInstance().GameNotifyFortressState(factionid,state);
	}
};

};

#endif
