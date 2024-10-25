
#ifndef __GNET_NOTIFYFACTIONFORTRESSINFO2_HPP
#define __GNET_NOTIFYFACTIONFORTRESSINFO2_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gfactionfortressinfo2"

namespace GNET
{

class NotifyFactionFortressInfo2 : public GNET::Protocol
{
	#include "notifyfactionfortressinfo2"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
