
#ifndef __GNET_FACTIONBEGINSYNC_RE_HPP
#define __GNET_FACTIONBEGINSYNC_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "factionopsyncinfo"
namespace GNET
{

class FactionBeginSync_Re : public GNET::Protocol
{
	#include "factionbeginsync_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
