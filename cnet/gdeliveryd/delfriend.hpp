
#ifndef __GNET_DELFRIEND_HPP
#define __GNET_DELFRIEND_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "delfriend_re.hpp"
#include "mapuser.h"
#include "gametalkmanager.h"
#include "gametalkdefs.h"

namespace GNET
{

class DelFriend : public GNET::Protocol
{
	#include "delfriend"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		char ret = ERR_FS_NOFOUND;
		GDeliveryServer* dsm = GDeliveryServer::GetInstance();
		{
			Thread::RWLock::WRScoped l(UserContainer::GetInstance().GetLocker());
			PlayerInfo * pinfo = UserContainer::GetInstance().FindRole((roleid));
			if( NULL==pinfo )
				return;
			GFriendInfoVector* plist = &(pinfo->friends);
			GFriendExtInfoVector* pextlist = &(pinfo->friendextinfo);
			if(pinfo->friend_ver>=0)
			{
				for(GFriendInfoVector::iterator itf=plist->begin(), ite=plist->end(); itf!=ite; ++itf)
				{
					if(itf->rid==friendid)
					{
						plist->erase(itf);
						ret = 0;
						pinfo->friend_ver++;
						GameTalkManager::GetInstance()->NotifyUpdateFriend(roleid, GT_FRIEND_DEL, friendid);
						break;
					}
				}
				for(GFriendExtInfoVector::iterator itfe=pextlist->begin();itfe!=pextlist->end();++itfe)
				{
					if(itfe->rid==friendid)
					{
						pextlist->erase(itfe);
						break;
					}
				}
			}
			else
				ret = ERR_FS_NOTINITIALIZED;
		}
		dsm->Send(sid, DelFriend_Re(ret, roleid, friendid, localsid));
	}
};

};

#endif
