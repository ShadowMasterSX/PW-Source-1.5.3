
#ifndef __GNET_AUTOTEAMCOMPOSESTART_HPP
#define __GNET_AUTOTEAMCOMPOSESTART_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class AutoTeamComposeStart : public GNET::Protocol
{
	#include "autoteamcomposestart"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
