
#ifndef __GNET_FACTIONMEMBERUPDATE_HPP
#define __GNET_FACTIONMEMBERUPDATE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "factionidbean"
#include "rolebean"

namespace GNET
{

class FactionMemberUpdate : public GNET::Protocol
{
	#include "factionmemberupdate"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
