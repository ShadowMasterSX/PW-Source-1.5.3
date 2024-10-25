
#ifndef __GNET_GAMESYSMSG_HPP
#define __GNET_GAMESYSMSG_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class GameSysMsg : public GNET::Protocol
{
	#include "gamesysmsg"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
