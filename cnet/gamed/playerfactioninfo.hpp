
#ifndef __GNET_PLAYERFACTIONINFO_HPP
#define __GNET_PLAYERFACTIONINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class PlayerFactionInfo : public GNET::Protocol
{
	#include "playerfactioninfo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
