
#ifndef __GNET_REMOTELOG_HPP
#define __GNET_REMOTELOG_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class RemoteLog : public GNET::Protocol
{
	#include "remotelog"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
