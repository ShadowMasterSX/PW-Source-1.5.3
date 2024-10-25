#ifndef __GNET_DELETEROLE_TASK_H
#define __GNET_DELETEROLE_TASK_H

#include "thread.h"
#include "gamedbclient.hpp"
#include "delroleannounce.hpp"
namespace GNET
{
	class DeleteRoleTask : public Thread::Runnable
	{
		Octets key_cur;
		int update_time;
		DeleteRoleTask(int _update,int prior=1) : Runnable(prior), update_time(_update) { }
	public:
		~DeleteRoleTask() { }
		static DeleteRoleTask* GetInstance(int _update,int prior=1) 
		{ static DeleteRoleTask instance(_update,prior); return &instance; }
		void Run()
		{
			static int count = 0;
			if (count++ % 10 == 0)
			{
				// intevel = 30 , per 5 minutes
				int total_online = UserContainer::GetInstance().Size();
				int remote_online = UserContainer::GetInstance().RemoteOnlineSize();
				STAT_MIN5("OnlineUsers",(total_online - remote_online > 0 ? total_online - remote_online : 0));
				STAT_MIN5("RemoteOnlineUsers", remote_online);//¿ç·þÔÚÏß
				STAT_MIN5("AllOnlineUsers", total_online);
			}
			GameDBClient::GetInstance()->SendProtocol(DelRoleAnnounce(IntVector()));
			Thread::HouseKeeper::AddTimerTask(this,update_time);
		}
	};
};
#endif
