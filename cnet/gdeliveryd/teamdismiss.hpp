
#ifndef __GNET_TEAMDISMISS_HPP
#define __GNET_TEAMDISMISS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class TeamDismiss : public GNET::Protocol
{
	#include "teamdismiss"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
