
#ifndef __GNET_ROLEINFORESP_HPP
#define __GNET_ROLEINFORESP_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "roleinfobean"

namespace GNET
{

class RoleInfoResp : public GNET::Protocol
{
	#include "roleinforesp"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
