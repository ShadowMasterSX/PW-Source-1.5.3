
#ifndef __GNET_DBFRIENDEXTLIST_RE_HPP
#define __GNET_DBFRIENDEXTLIST_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gfriendextinfo"

namespace GNET
{

class DBFriendExtList_Re : public GNET::Protocol
{
	#include "dbfriendextlist_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Thread::RWLock::WRScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(rid);
		if (NULL == pinfo || pinfo->roleid!=rid)
			return;
		if(pinfo->friend_ver < 0)
			return;
		if(retcode == 0)
		{
			FriendextinfoManager::GetInstance()->UpdateRoleLoginTime(friendext,pinfo);
			return;
		}
	}
};

};

#endif
