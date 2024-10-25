
#ifndef __GNET_SERVERFORBIDNOTIFY_HPP
#define __GNET_SERVERFORBIDNOTIFY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class ServerForbidNotify : public GNET::Protocol
{
	#include "serverforbidnotify"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
