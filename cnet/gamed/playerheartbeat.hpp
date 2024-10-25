
#ifndef __GNET_PLAYERHEARTBEAT_HPP
#define __GNET_PLAYERHEARTBEAT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class PlayerHeartBeat : public GNET::Protocol
{
	#include "playerheartbeat"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
