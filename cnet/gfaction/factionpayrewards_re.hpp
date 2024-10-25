
#ifndef __GNET_FACTIONPAYREWARDS_RE_HPP
#define __GNET_FACTIONPAYREWARDS_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class FactionPayRewards_Re : public GNET::Protocol
{
	#include "factionpayrewards_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
