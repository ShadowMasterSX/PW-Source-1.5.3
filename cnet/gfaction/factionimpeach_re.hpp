
#ifndef __GNET_FACTIONIMPEACH_RE_HPP
#define __GNET_FACTIONIMPEACH_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class FactionImpeach_Re : public GNET::Protocol
{
	#include "factionimpeach_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
