
#ifndef __GNET_ROLEINFOUPDATE_HPP
#define __GNET_ROLEINFOUPDATE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "roleinfobean"

namespace GNET
{

class RoleInfoUpdate : public GNET::Protocol
{
	#include "roleinfoupdate"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
