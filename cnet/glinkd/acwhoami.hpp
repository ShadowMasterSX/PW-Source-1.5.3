
#ifndef __GNET_ACWHOAMI_HPP
#define __GNET_ACWHOAMI_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class ACWhoAmI : public GNET::Protocol
{
	#include "acwhoami"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
