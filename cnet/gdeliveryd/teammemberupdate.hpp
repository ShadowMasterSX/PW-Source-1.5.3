
#ifndef __GNET_TEAMMEMBERUPDATE_HPP
#define __GNET_TEAMMEMBERUPDATE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class TeamMemberUpdate : public GNET::Protocol
{
	#include "teammemberupdate"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
