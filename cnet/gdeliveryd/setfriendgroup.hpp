
#ifndef __GNET_SETFRIENDGROUP_HPP
#define __GNET_SETFRIENDGROUP_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "setfriendgroup_re.hpp"
#include "mapuser.h"
#include "gametalkmanager.h"
#include "gametalkdefs.h"

namespace GNET
{

class SetFriendGroup : public GNET::Protocol
{
	#include "setfriendgroup"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		char ret = ERR_FS_NOFOUND;
		GDeliveryServer* dsm = GDeliveryServer::GetInstance();
		{
			Thread::RWLock::WRScoped l(UserContainer::GetInstance().GetLocker());
			PlayerInfo * pinfo = UserContainer::GetInstance().FindRole((roleid));
			if( NULL==pinfo || pinfo->friend_ver<0)
				return;
			GFriendInfoVector* plist = &(pinfo->friends);
			for(GFriendInfoVector::iterator itf = plist->begin(), ite=plist->end();itf!=ite;++itf)
			{
				if(itf->rid==friendid)
				{
					itf->gid = gid;
					pinfo->friend_ver++;
					ret = 0;
					GameTalkManager::GetInstance()->NotifyUpdateFriend(roleid, GT_FRIEND_MOVE, friendid, gid);
					break;
				}
			}
		}
		dsm->Send(sid, SetFriendGroup_Re(ret, gid, roleid, friendid, localsid));
	}
};

};

#endif
