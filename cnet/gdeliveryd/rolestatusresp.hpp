
#ifndef __GNET_ROLESTATUSRESP_HPP
#define __GNET_ROLESTATUSRESP_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gametalkmanager.h"

namespace GNET
{

class RoleStatusResp : public GNET::Protocol
{
	#include "rolestatusresp"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GameTalkManager::GetInstance()->GTRoleResponse(rolestatus);
	}
};

};

#endif
