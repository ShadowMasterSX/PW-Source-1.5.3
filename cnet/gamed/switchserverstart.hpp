
#ifndef __GNET_SWITCHSERVERSTART_HPP
#define __GNET_SWITCHSERVERSTART_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gproviderclient.hpp"
#ifdef _TESTCODE
	#include "switchserversuccess.hpp"
#endif

void switch_server_request(int link_id,int user_id, int localsid, int source,const void * key_buf, size_t key_size);
namespace GNET
{
#ifdef _TESTCODE
	/*
void* switch_response(void* p)
{
	sleep(5);
	GProviderClient::GetInstance()->DispatchProtocol(0,(Protocol*)p);
	GProviderClient::GetInstance()->DispatchProtocol(((SwitchServerSuccess*)p)->link_id,(Protocol*)p);
	//DEBUG_PRINT("gamed::send switchserversuccess. link_id=%d,roleid=%d\n",((SwitchServerSuccess*)p)->link_id,((SwitchServerSuccess*)p)->roleid);
	((Protocol*)p)->Destroy();
	PollIO::WakeUp();
	return NULL;
}
	*/
#endif
class SwitchServerStart : public GNET::Protocol
{
	#include "switchserverstart"
	class SwitchServerStartTask : public Thread::Runnable
	{
		int roleid;
		int link_id;
		int localsid;
		int src_gsid;
		int dst_gsid;
		Octets key;
	public:
		SwitchServerStartTask(int _r,int _lid, int _sid,int _src,int _dst,Octets & _key) : Runnable(1), roleid(_r),link_id(_lid),localsid(_sid),src_gsid(_src),dst_gsid(_dst),key(_key) { }
		void Run()
		{
			switch_server_request(link_id,roleid,localsid,src_gsid,key.begin(),key.size());
			delete this;
		}	
	};
	void Process(Manager *manager, Manager::Session::ID sid)
	{
#ifdef _TESTCODE
		// TODO
		DEBUG_PRINT("gamed(%d): receive switchserverstart, switching.....,please wait 5 seconds.\n",GProviderClient::GetGameServerID());
		//pthread_t thd;
		//pthread_create(&thd,NULL,switch_response,new SwitchServerSuccess(roleid,link_id,localsid,2));
#endif
		Thread::Pool::AddTask(new SwitchServerStartTask(roleid,link_id,localsid,src_gsid,dst_gsid,key));
	}
};

};

#endif
