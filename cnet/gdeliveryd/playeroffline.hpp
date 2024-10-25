
#ifndef __GNET_PLAYEROFFLINE_HPP
#define __GNET_PLAYEROFFLINE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class PlayerOffline : public GNET::Protocol
{
	#include "playeroffline"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
