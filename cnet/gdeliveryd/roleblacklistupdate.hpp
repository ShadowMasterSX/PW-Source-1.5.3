
#ifndef __GNET_ROLEBLACKLISTUPDATE_HPP
#define __GNET_ROLEBLACKLISTUPDATE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "roleinfobean"

namespace GNET
{

class RoleBlacklistUpdate : public GNET::Protocol
{
	#include "roleblacklistupdate"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
