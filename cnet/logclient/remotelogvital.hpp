
#ifndef __GNET_REMOTELOGVITAL_HPP
#define __GNET_REMOTELOGVITAL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class RemoteLogVital : public GNET::Protocol
{
	#include "remotelogvital"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
