
#ifndef __GNET_SERVERTRIGGERNOTIFY_HPP
#define __GNET_SERVERTRIGGERNOTIFY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class ServerTriggerNotify : public GNET::Protocol
{
	#include "servertriggernotify"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
