
#ifndef __GNET_SERVERTRIGGERNOTIFY_HPP
#define __GNET_SERVERTRIGGERNOTIFY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void notify_servertrigger(std::vector<int> &trigger_list);

namespace GNET
{

class ServerTriggerNotify : public GNET::Protocol
{
	#include "servertriggernotify"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		notify_servertrigger(trigger_ctrl_list);
	}
};

};

#endif
