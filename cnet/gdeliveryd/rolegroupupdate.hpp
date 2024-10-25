
#ifndef __GNET_ROLEGROUPUPDATE_HPP
#define __GNET_ROLEGROUPUPDATE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class RoleGroupUpdate : public GNET::Protocol
{
	#include "rolegroupupdate"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
