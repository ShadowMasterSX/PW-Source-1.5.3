
#ifndef __GNET_TEAMCREATE_HPP
#define __GNET_TEAMCREATE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "teambean"

namespace GNET
{

class TeamCreate : public GNET::Protocol
{
	#include "teamcreate"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
