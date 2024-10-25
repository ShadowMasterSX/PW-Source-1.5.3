
#ifndef __GNET_AUTOTEAMCOMPOSESTART_HPP
#define __GNET_AUTOTEAMCOMPOSESTART_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void autoteam_compose_start(int goal_id, int roleid, int member_list[], unsigned int cnt);

namespace GNET
{

class AutoTeamComposeStart : public GNET::Protocol
{
	#include "autoteamcomposestart"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		autoteam_compose_start(goal_id, roleid, &(*member_list.begin()), member_list.size());
	}
};

};

#endif
