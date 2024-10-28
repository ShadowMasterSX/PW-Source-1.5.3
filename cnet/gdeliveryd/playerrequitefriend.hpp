
#ifndef __GNET_PLAYERREQUITEFRIEND_HPP
#define __GNET_PLAYERREQUITEFRIEND_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class PlayerRequiteFriend : public GNET::Protocol
{
	#include "playerrequitefriend"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Thread::RWLock::WRScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if (NULL == pinfo || pinfo->roleid!=roleid)
			return;
		if(pinfo->friend_ver < 0)
			return;
		FriendextinfoManager::GetInstance()->PreSendRequite(pinfo,friendid);
	}
};

};

#endif
