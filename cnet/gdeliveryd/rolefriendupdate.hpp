
#ifndef __GNET_ROLEFRIENDUPDATE_HPP
#define __GNET_ROLEFRIENDUPDATE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "rolebean"

namespace GNET
{

class RoleFriendUpdate : public GNET::Protocol
{
	#include "rolefriendupdate"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
