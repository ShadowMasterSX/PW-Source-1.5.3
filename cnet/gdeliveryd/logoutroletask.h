#ifndef __LOGOUTROLE_TASK_H
#define __LOGOUTROLE_TASK_H
#include "thread.h"
#include "ggroupinfo"
#include "gfriendinfo"
#include "friendstatus.hpp"
#include "friendlistpair"
#include "putfriendlist.hrp"
#include "chatroom.h"
#include "mapuser.h"
#include "stockexchange.h"
#include "gametalkmanager.h"

namespace GNET
{
	class LogoutRoleTask : public Thread::Runnable
	{
		int roleid;
		int friend_ver;
		GGroupInfoVector  groups;
		GFriendInfoVector friends; 
		GFriendExtInfoVector friendextinfo;
		GSendAUMailRecordVector sendaumailinfo;
	public:
		LogoutRoleTask(PlayerInfo& user)
		{
			roleid = user.roleid;
			friend_ver = user.friend_ver;
			if(user.friend_ver>=0)
			{
				groups.swap(user.groups);
				friends.swap(user.friends);
				friendextinfo.swap(user.friendextinfo);
				sendaumailinfo.swap(user.sendaumailinfo);
			}
		}
		~LogoutRoleTask(){}

		void Run()
		{
			RoomManager::GetInstance()->ResetRole(roleid);

			FriendStatus stat(roleid, GameTalkManager::GetInstance()->GetRoleStatus(roleid), 0);
			
			GDeliveryServer* dsm = GDeliveryServer::GetInstance();
			{
				for(GFriendInfoVector::iterator it = friends.begin();it!=friends.end();it++)
				{
					PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(it->rid);
					if (pinfo)
					{
						stat.localsid = pinfo->localsid;
						dsm->Send(pinfo->linksid,stat);
					}
				}
			}
			if(friend_ver>0)
			{
				FriendListPair pair;
				pair.key = roleid;
				pair.value.groups.swap(groups); 
				pair.value.friends.swap(friends); 
				pair.extra_info.swap(friendextinfo);
				pair.sendmail_info.swap(sendaumailinfo);
				PutFriendList* rpc = (PutFriendList*) Rpc::Call(RPC_PUTFRIENDLIST,pair);
				GameDBClient::GetInstance()->SendProtocol(rpc);
			}
			delete this;
		}

		static void Add(PlayerInfo& user)
		{
			if(user.roleid != 0)
				Thread::Pool::AddTask(new LogoutRoleTask(user));
		}
	};
}
#endif
