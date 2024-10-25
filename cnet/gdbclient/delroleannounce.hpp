
#ifndef __GNET_DELROLEANNOUNCE_HPP
#define __GNET_DELROLEANNOUNCE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class DelRoleAnnounce : public GNET::Protocol
{
	#include "delroleannounce"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
