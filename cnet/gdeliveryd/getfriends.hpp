
#ifndef __GNET_GETFRIENDS_HPP
#define __GNET_GETFRIENDS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "getfriends_re.hpp"
#include "mapuser.h"
#include "gametalkmanager.h"
#include "namemanager.h"
namespace GNET
{

class GetFriends : public GNET::Protocol
{
	#include "getfriends"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GetFriends_Re re;
		GDeliveryServer* dsm = GDeliveryServer::GetInstance();
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());	
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRole((roleid));
		if( NULL == pinfo )
			return;
		if(pinfo->friend_ver<0)
		{
			DEBUG_PRINT("gdelivery::getfriends: roleid=%d.\n", roleid);
			GetFriendList* rpc = (GetFriendList*) Rpc::Call(RPC_GETFRIENDLIST,RoleId(roleid));
			rpc->roleid = roleid;
			GameDBClient::GetInstance()->SendProtocol(rpc);
			return;
		}
		else
		{
			re.groups = pinfo->groups;
			re.friends = pinfo->friends;
			
			GameTalkManager *gtm = GameTalkManager::GetInstance();
			GFriendInfoVector* plist = &(pinfo->friends);
			char counter = 0;
			Octets name;
			for(GFriendInfoVector::iterator i = plist->begin();i!=plist->end();i++)
			{
				//玩家的好友可能已经改过名字了，在修改记录里查找一下
				if (NameManager::GetInstance()->FindName(i->rid, name))
				{
					i->name = name;
				}
				PlayerInfo * ruser = UserContainer::GetInstance().FindRoleOnline((i->rid));
				if(NULL != ruser)
				{
					for(GFriendInfoVector::iterator k=ruser->friends.begin(),ke=ruser->friends.end();k!=ke;++k)
					{
						if(k->rid==roleid)
						{
							re.status.push_back(counter);
							re.status.push_back(gtm->GetRoleStatus(i->rid));
							break;
						}
					}
				} else {
					char s = gtm->GetRoleStatus(i->rid);
					if(s != RoleStatusManager::GT_OFFLINE) {
						re.status.push_back(counter);
						re.status.push_back(s);
					}
				}
				counter++;
			}
		}
		re.roleid = roleid;
		re.localsid = localsid;
		dsm->Send(sid, re);
		//取好友最近上线时间
		FriendextinfoManager::GetInstance()->SearchAllExt(pinfo);	
	}
};

};

#endif
