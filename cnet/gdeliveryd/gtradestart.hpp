
#ifndef __GNET_GTRADESTART_HPP
#define __GNET_GTRADESTART_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class GTradeStart : public GNET::Protocol
{
	#include "gtradestart"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
