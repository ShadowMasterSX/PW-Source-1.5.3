
#ifndef __GNET_FACTIONENDSYNC_HPP
#define __GNET_FACTIONENDSYNC_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "factionopsyncinfo"
namespace GNET
{

class FactionEndSync : public GNET::Protocol
{
	#include "factionendsync"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
