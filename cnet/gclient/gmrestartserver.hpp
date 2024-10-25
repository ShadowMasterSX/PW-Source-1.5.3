
#ifndef __GNET_GMRESTARTSERVER_HPP
#define __GNET_GMRESTARTSERVER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class GMRestartServer : public GNET::Protocol
{
	#include "gmrestartserver"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		printf("服务器将在%d秒后关闭，请及时退出......\n",restart_time);
	}
};

};

#endif
