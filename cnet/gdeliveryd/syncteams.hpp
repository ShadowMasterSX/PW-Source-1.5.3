
#ifndef __GNET_SYNCTEAMS_HPP
#define __GNET_SYNCTEAMS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "teambean"

namespace GNET
{

class SyncTeams : public GNET::Protocol
{
	#include "syncteams"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
