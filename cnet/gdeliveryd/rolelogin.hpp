
#ifndef __GNET_ROLELOGIN_HPP
#define __GNET_ROLELOGIN_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class RoleLogin : public GNET::Protocol
{
	#include "rolelogin"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
