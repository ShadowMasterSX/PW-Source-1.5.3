
#ifndef __GNET_PLAYERTEAMOP_HPP
#define __GNET_PLAYERTEAMOP_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "teambean"

namespace GNET
{

class PlayerTeamOp : public GNET::Protocol
{
	#include "playerteamop"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
