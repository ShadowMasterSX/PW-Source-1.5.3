
#ifndef __GNET_SETGROUPNAME_HPP
#define __GNET_SETGROUPNAME_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "setgroupname_re.hpp"
#include "mapuser.h"
#include "gametalkmanager.h"
#include "gametalkdefs.h"

namespace GNET
{

class SetGroupName : public GNET::Protocol
{
	#include "setgroupname"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		char ret = ERR_FS_NOFOUND;
		if(name.size()>20||gid==0)
			ret = ERR_FS_ERRPARAM;
		GDeliveryServer* dsm = GDeliveryServer::GetInstance();
		if(ret==ERR_FS_NOFOUND)
		{
			Thread::RWLock::WRScoped l(UserContainer::GetInstance().GetLocker());
			PlayerInfo * pinfo = UserContainer::GetInstance().FindRole((roleid));
			if( NULL==pinfo || pinfo->friend_ver<0)
				return;
			GGroupInfoVector* plist = &(pinfo->groups);
			for(GGroupInfoVector::iterator itg = plist->begin(), itge=plist->end();itg!=itge;++itg)
			{
				if(itg->gid!=gid)
					continue;
				ret=0;
				if(name.size()==0)
				{
					plist->erase(itg);
					GFriendInfoVector* flist = &(pinfo->friends);
					for(GFriendInfoVector::iterator itf = flist->begin(), itfe=flist->end();itf!=itfe;++itf)
					{
						if(itf->gid==gid)
							itf->gid = 0;
					}
					GameTalkManager::GetInstance()->NotifyUpdateGroup(roleid, GT_GROUP_DEL, gid);
				}
				else {
					itg->name = name;
					GameTalkManager::GetInstance()->NotifyUpdateGroup(roleid, GT_GROUP_MOD, gid, GT_GROUP_NORMAL, name);
				}
				break;
			}
			if(ret==ERR_FS_NOFOUND)
			{
				if(plist->size()>=10)
					ret = ERR_FS_NOSPACE;
				else
				{
					plist->push_back(GGroupInfo(gid,name));
					GameTalkManager::GetInstance()->NotifyUpdateGroup(roleid, GT_GROUP_ADD, gid, GT_GROUP_NORMAL, name);
					ret = 0;
				}
			}
			if(ret==0)
				pinfo->friend_ver++;
		}
		dsm->Send(sid, SetGroupName_Re(ret, roleid, gid, name, localsid));
	}
};

};

#endif
