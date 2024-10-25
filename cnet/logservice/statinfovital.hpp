
#ifndef __GNET_STATINFOVITAL_HPP
#define __GNET_STATINFOVITAL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "logdispatch.h"

namespace GNET
{

class StatInfoVital : public GNET::Protocol
{
	#include "statinfovital"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GNET::LogDispatch::stat( priority, msg, hostname, servicename );
	}
};

};

#endif
