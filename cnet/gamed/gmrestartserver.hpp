
#ifndef __GNET_GMRESTARTSERVER_HPP
#define __GNET_GMRESTARTSERVER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void gm_shutdown_server();

namespace GNET
{

class GMRestartServer : public GNET::Protocol
{
	#include "gmrestartserver"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		//printf("!!!!!!!!!!!!!!!GM %d restart server in %d seconds!!!!!!!!!!!!!!!!!!!!!\n",gmroleid,restart_time);
		gm_shutdown_server();
	}
};

};

#endif
