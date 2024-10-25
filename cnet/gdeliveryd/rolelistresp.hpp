
#ifndef __GNET_ROLELISTRESP_HPP
#define __GNET_ROLELISTRESP_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "roledetailbean"

namespace GNET
{

class RoleListResp : public GNET::Protocol
{
	#include "rolelistresp"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
