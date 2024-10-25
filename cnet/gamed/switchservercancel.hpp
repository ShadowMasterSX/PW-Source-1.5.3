
#ifndef __GNET_SWITCHSERVERCANCEL_HPP
#define __GNET_SWITCHSERVERCANCEL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void switch_server_cancel(int link_id,int user_id, int localsid, int source = 0);
namespace GNET
{

class SwitchServerCancel : public GNET::Protocol
{
	#include "switchservercancel"
	class SwitchServerStartTask : public Thread::Runnable
	{
		int roleid;
		int link_id;
		int localsid;
	public:
		SwitchServerStartTask(int _r,int _lid, int _sid) : Runnable(1), roleid(_r),link_id(_lid),localsid(_sid) { }
		void Run()
		{
			delete this;
		}	
	};

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		switch_server_cancel(link_id,roleid,localsid);
	}
};

};

#endif
