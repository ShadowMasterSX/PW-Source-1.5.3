
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
		printf("����������%d���رգ��뼰ʱ�˳�......\n",restart_time);
	}
};

};

#endif
