
#ifndef __GNET_QUERYGAMESERVERATTR_HPP
#define __GNET_QUERYGAMESERVERATTR_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class QueryGameServerAttr : public GNET::Protocol
{
	#include "querygameserverattr"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
