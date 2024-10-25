
#ifndef __GNET_GMRESTARTSERVER_HPP
#define __GNET_GMRESTARTSERVER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gdeliveryserver.hpp"
#include "gproviderserver.hpp"
#include "mapforbid.h"
#include "maplinkserver.h"
namespace GNET
{

class GMRestartServer : public GNET::Protocol
{
	#include "gmrestartserver"
	class GMRestartTask : public Thread::Runnable
	{
		static Thread::Mutex locker;
		static GMRestartTask instance;
		int roleid;
		int restart_time;		//重启倒计时时间
		int forbid_login;		//离重启多长时间，禁止登陆
		int announce_interval;	//通告客户端的时间间隔
		bool running;			//重启任务是否已经执行，如果已经执行，不允许添加新的重启任务
		GMRestartTask() { running=false; }
		GMRestartTask(int _roleid,int _restart=300,int _forbid=60,int _interval=60,bool _running=false) : Runnable(1), roleid(_roleid),restart_time(_restart),forbid_login(_forbid),announce_interval(_interval),running(_running)
		{ }
	public:
		~GMRestartTask() { }
		static void ForbidLogin()
		{
			DEBUG_PRINT("!!!!!!!!!!!!!!!!gdlivery::GMRestartTask:: Forbid all users to login!!!!!!!!!!!!!!!\n");
			ForbidLogin::GetInstance().ForbidLoginGlobal();
		}
		static void AllowLogin()
		{
			DEBUG_PRINT("!!!!!!!!!!!!!!!!gdlivery::GMRestartTask:: Allow all users to login!!!!!!!!!!!!!!!\n");
			ForbidLogin::GetInstance().AllowLoginGlobal();
		}
		static void AnnounceGameServer()
		{
			DEBUG_PRINT("!!!!!!!!!!!!!!!!gdlivery::GMRestartTask:: Announce GameServer to restart,gmroleid=%d,restart_time=%d!!!!!!!!!!!!!!!\n",instance.roleid,instance.restart_time);
			GProviderServer::GetInstance()->BroadcastProtocol(GMRestartServer(instance.roleid,0,0,instance.restart_time));
		}
		static void AnnounceClient()
		{
			DEBUG_PRINT("!!!!!!!!!!!!!!!!gdlivery::GMRestartTask:: Announce Client restart in %d second!!!!!!!!!!!!!!!\n",instance.restart_time);
			LinkServer::GetInstance().BroadcastProtocol(GMRestartServer(instance.roleid,0,0,instance.restart_time));
		}
		static bool StartTask(int gmroleid,int _restart=300,int _forbid=60,int _interval=20) {
			Thread::Mutex::Scoped l(locker);
			if (!instance.running)
			{
				instance.roleid=gmroleid;
				if (_restart<_interval) _restart=_interval;
				if (_forbid>_restart) _forbid=_restart;
				instance.announce_interval=_interval;
				instance.restart_time=_restart;
				instance.forbid_login=_forbid;
				instance.running=true;
				Thread::HouseKeeper::AddTimerTask(&instance,_interval>_restart ? _restart : _interval);
				return true;
			}
			return false;
		}
		static bool IsRunning()
		{ 
			Thread::Mutex::Scoped l(locker);
			return instance.running; 
		}
		static void ChangeParam(int _interval,int _restart,int _forbid)
		{
			Thread::Mutex::Scoped l(locker);
			instance.announce_interval=_interval;
			instance.restart_time=_restart;
			instance.forbid_login=_forbid;
		}
		void Run()
		{
			Thread::Mutex::Scoped l(locker);
			instance.restart_time-=instance.announce_interval;
			if (instance.restart_time<=0)
			{
				ForbidLogin();
				AnnounceGameServer();
				running=false;
			}
			else
			{
				AnnounceClient();
				if (instance.restart_time<=instance.forbid_login || 
						instance.restart_time-instance.announce_interval<10) ForbidLogin();
				Thread::HouseKeeper::AddTimerTask(this,instance.restart_time>instance.announce_interval ? instance.announce_interval:instance.restart_time);
			}
		}

	};
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("gdelivery::gmrestartserver:: received. gmroleid=%d, restart_time=%d\n",gmroleid,restart_time);
		//add restart task to task-pool
		if (GMRestartTask::StartTask(gmroleid,restart_time))
		{
			//announce to all linkserver
			LinkServer::GetInstance().BroadcastProtocol(GMRestartServer(gmroleid,0,0,restart_time));
		}
	}
};

};

#endif
