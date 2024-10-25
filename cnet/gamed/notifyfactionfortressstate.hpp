
#ifndef __GNET_NOTIFYFACTIONFORTRESSSTATE_HPP
#define __GNET_NOTIFYFACTIONFORTRESSSTATE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class NotifyFactionFortressState : public GNET::Protocol
{
	#include "notifyfactionfortressstate"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
