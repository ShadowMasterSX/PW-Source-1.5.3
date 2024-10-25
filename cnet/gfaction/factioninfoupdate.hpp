
#ifndef __GNET_FACTIONINFOUPDATE_HPP
#define __GNET_FACTIONINFOUPDATE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "factionidbean"
#include "factioninfobean"

namespace GNET
{

class FactionInfoUpdate : public GNET::Protocol
{
	#include "factioninfoupdate"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
