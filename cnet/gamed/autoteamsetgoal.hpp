
#ifndef __GNET_AUTOTEAMSETGOAL_HPP
#define __GNET_AUTOTEAMSETGOAL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class AutoTeamSetGoal : public GNET::Protocol
{
	#include "autoteamsetgoal"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
