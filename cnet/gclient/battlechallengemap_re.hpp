
#ifndef __GNET_BATTLECHALLENGEMAP_RE_HPP
#define __GNET_BATTLECHALLENGEMAP_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gbattlechallenge"

namespace GNET
{

class BattleChallengeMap_Re : public GNET::Protocol
{
	#include "battlechallengemap_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
