
#ifndef __GNET_FACTIONMEMBERUPDATE_HPP
#define __GNET_FACTIONMEMBERUPDATE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "factionidbean"
#include "rolebean"
#include "gametalkmanager.h"

namespace GNET
{

class FactionMemberUpdate : public GNET::Protocol
{
	#include "factionmemberupdate"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		RoleBeanVector::iterator it = roles.begin();
		RoleBeanVector::iterator end = roles.end();
		GameTalkManager *gtm = GameTalkManager::GetInstance();
		for(; it != end; ++it)
			it->status.status = gtm->GetRoleStatus((int)it->info.roleid);
		GameTalkManager::GetInstance()->NotifyUpdate(*this);
	}
};

};

#endif
