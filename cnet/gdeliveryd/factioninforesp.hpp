
#ifndef __GNET_FACTIONINFORESP_HPP
#define __GNET_FACTIONINFORESP_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "factionidbean"
#include "factioninfobean"
#include "factiontitlebean"

namespace GNET
{

class FactionInfoResp : public GNET::Protocol
{
	#include "factioninforesp"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
