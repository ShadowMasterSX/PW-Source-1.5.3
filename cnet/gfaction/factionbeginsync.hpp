
#ifndef __GNET_FACTIONBEGINSYNC_HPP
#define __GNET_FACTIONBEGINSYNC_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class FactionBeginSync : public GNET::Protocol
{
	#include "factionbeginsync"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
