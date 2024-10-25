
#ifndef __GNET_STATINFO_HPP
#define __GNET_STATINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "logdispatch.h"

namespace GNET
{

class StatInfo : public GNET::Protocol
{
	#include "statinfo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GNET::LogDispatch::stat( priority, msg, hostname, servicename );
	}
};

};

#endif
