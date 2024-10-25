
#ifndef __GNET_FACTIONMSG_HPP
#define __GNET_FACTIONMSG_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "factionidbean"
#include "rolemsgbean"

namespace GNET
{

class FactionMsg : public GNET::Protocol
{
	#include "factionmsg"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
