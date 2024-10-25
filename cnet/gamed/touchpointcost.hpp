
#ifndef __GNET_TOUCHPOINTCOST_HPP
#define __GNET_TOUCHPOINTCOST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gtouchtrade"

namespace GNET
{

class TouchPointCost : public GNET::Protocol
{
	#include "touchpointcost"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
