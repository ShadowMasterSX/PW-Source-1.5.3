
#ifndef __GNET_AUTOTEAMPLAYERREADY_HPP
#define __GNET_AUTOTEAMPLAYERREADY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class AutoTeamPlayerReady : public GNET::Protocol
{
	#include "autoteamplayerready"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
