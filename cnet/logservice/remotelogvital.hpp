
#ifndef __GNET_REMOTELOGVITAL_HPP
#define __GNET_REMOTELOGVITAL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "logdispatch.h"

namespace GNET
{

class RemoteLogVital : public GNET::Protocol
{
	#include "remotelogvital"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GNET::LogDispatch::log( priority, msg, hostname, servicename );
	}
};

};

#endif
