
#ifndef __GNET_GTRADEDISCARD_HPP
#define __GNET_GTRADEDISCARD_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class GTradeDiscard : public GNET::Protocol
{
	#include "gtradediscard"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
