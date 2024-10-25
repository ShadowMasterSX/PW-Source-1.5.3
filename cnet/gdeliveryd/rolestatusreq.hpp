
#ifndef __GNET_ROLESTATUSREQ_HPP
#define __GNET_ROLESTATUSREQ_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gametalkmanager.h"

namespace GNET
{

class RoleStatusReq : public GNET::Protocol
{
	#include "rolestatusreq"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GameTalkManager::GetInstance()->GTRoleRequest(rolelist, localrid);
	}
};

};

#endif
