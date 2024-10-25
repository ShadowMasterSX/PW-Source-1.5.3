
#ifndef __GNET_ROLESTATUSUPDATE_HPP
#define __GNET_ROLESTATUSUPDATE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "rolestatusbean"
#include "factionidbean"

namespace GNET
{

class RoleStatusUpdate : public GNET::Protocol
{
	#include "rolestatusupdate"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GameTalkManager::GetInstance()->GTRoleUpdate(roleid, status.status, friends);
	}
};

};

#endif
