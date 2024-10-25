
#ifndef __GNET_PLAYERSTATUSSYNC_HPP
#define __GNET_PLAYERSTATUSSYNC_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class PlayerStatusSync : public GNET::Protocol
{
	#include "playerstatussync"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
