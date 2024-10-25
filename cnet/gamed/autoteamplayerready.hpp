
#ifndef __GNET_AUTOTEAMPLAYERREADY_HPP
#define __GNET_AUTOTEAMPLAYERREADY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


void autoteam_player_ready(int roleid, int leader_id);

namespace GNET
{

class AutoTeamPlayerReady : public GNET::Protocol
{
	#include "autoteamplayerready"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		autoteam_player_ready(roleid, leader_id);
	}
};

};

#endif
