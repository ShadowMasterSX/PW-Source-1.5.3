
#ifndef __GNET_QUERYPLAYERSTATUS_HPP
#define __GNET_QUERYPLAYERSTATUS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class QueryPlayerStatus : public GNET::Protocol
{
	#include "queryplayerstatus"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
