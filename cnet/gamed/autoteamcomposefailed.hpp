
#ifndef __GNET_AUTOTEAMCOMPOSEFAILED_HPP
#define __GNET_AUTOTEAMCOMPOSEFAILED_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void autoteam_compose_failed(int roleid, int leader_id);

namespace GNET
{

class AutoTeamComposeFailed : public GNET::Protocol
{
	#include "autoteamcomposefailed"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		autoteam_compose_failed(roleid, leader_id);
	}
};

};

#endif
