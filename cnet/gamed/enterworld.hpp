
#ifndef __GNET_ENTERWORLD_HPP
#define __GNET_ENTERWORLD_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gproviderclient.hpp"
#include "s2cgamedatasend.hpp"	
#include "playerlogout.hpp"

#ifdef _TESTCODE
	#include "switchserverstart.hpp"
#endif
void user_enter_world(int cs_index,int sid,int uid,int locktime, int maxlocktime);
namespace GNET
{
#ifdef _TESTCODE
struct INFO
{
	int roleid;
	unsigned int localsid;
	INFO(int _r,unsigned int _l) : roleid(_r),localsid(_l) { }
};
void* _Sender(void* pInfo)
{
	INFO info=*((INFO*)pInfo);
	free(pInfo);
	printf("gamed::enterworld: enter sending-thread. roleid=%d,localsid=%d\n",info.roleid,info.localsid);
	S2CGamedataSend s2c(info.roleid,info.localsid,Octets("Hello",5));
	while(1)
	{
		GProviderClient::GetInstance()->DispatchProtocol(1,s2c);
		usleep(10000);
		PollIO::WakeUp();
	}
	return NULL;
}
class TestPlayerLogout : public Thread::Runnable
{
	int roleid;
	int provider_linkid;
	unsigned int localsid;
public:
	TestPlayerLogout(int _r,int _p,unsigned int _l,int prior = 1) : Runnable(prior),roleid(_r),provider_linkid(_p),localsid(_l) { }
	void Run()
	{
		DEBUG_PRINT("gamed::Send playlogout to gdelivery,retcode=_PLAYER_LOGOUT_HALF\n");
		GProviderClient::DispatchProtocol(0,PlayerLogout(_PLAYER_LOGOUT_HALF,roleid,provider_linkid,localsid));
		PollIO::WakeUp();
		delete this;
	}
};	

#endif
class EnterWorld : public GNET::Protocol
{
/*
	class EnterWorldTask : public Thread::Runnable
	{
		int provider_link_id;
		unsigned int localsid;
		int roleid;
	public:
		EnterWorldTask(int _p, int _l, int _r) : Runnable(1),provider_link_id(_p), localsid(_l),roleid(_r) { }
		void Run()
		{	
			user_enter_world(provider_link_id,localsid,roleid,0,0);	
			delete this;
		}
	};
	*/

	#include "enterworld"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
#ifdef _TESTCODE //liping
		DEBUG_PRINT("gamed(%d)::receive enterworld from delivery. roleid=%d\n",GProviderClient::GetGameServerID(),roleid);
		//pthread_t thd;
		//INFO* info=new INFO(roleid,localsid);
		//pthread_create(&thd,NULL,_Sender,info);
		//Thread::HouseKeeper::AddTimerTask(new TestPlayerLogout(roleid,provider_link_id,localsid),15);

		//test switchserver
		/*
		if (GProviderClient::GetGameServerID() == 1)
		{
			DEBUG_PRINT("gamed(%d): send switchserverstart to link and delivery server.\n",GProviderClient::GetGameServerID());
			//send both to delivery and link
			GProviderClient::GetInstance()->DispatchProtocol(0,SwitchServerStart(roleid,provider_link_id,localsid,1,2));
			GProviderClient::GetInstance()->DispatchProtocol(provider_link_id,SwitchServerStart(roleid,provider_link_id,localsid,1,2));
		}
		*/
#endif
		//Thread::Pool::AddTask(new EnterWorldTask(provider_link_id,localsid,roleid));
		user_enter_world(provider_link_id,localsid,roleid,timeout,locktime);
	}
};

};

#endif
