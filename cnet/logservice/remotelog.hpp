
#ifndef __GNET_REMOTELOG_HPP
#define __GNET_REMOTELOG_HPP

#include <time.h>
#include <fstream>
#include <iostream>

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "logdispatch.h"

namespace GNET
{

class RemoteLog : public GNET::Protocol
{
	#include "remotelog"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GNET::LogDispatch::log( priority, msg, hostname, servicename );
	}
};

};

#endif
