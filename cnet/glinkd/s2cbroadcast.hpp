
#ifndef __GNET_S2CBROADCAST_HPP
#define __GNET_S2CBROADCAST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class S2CBroadcast : public GNET::Protocol
{
	#include "s2cbroadcast"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
