
#ifndef __GNET_TOUCHPOINTQUERY_HPP
#define __GNET_TOUCHPOINTQUERY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class TouchPointQuery : public GNET::Protocol
{
	#include "touchpointquery"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
