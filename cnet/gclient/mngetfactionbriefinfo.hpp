
#ifndef __GNET_MNGETFACTIONBRIEFINFO_HPP
#define __GNET_MNGETFACTIONBRIEFINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class MNGetFactionBriefInfo : public GNET::Protocol
{
	#include "mngetfactionbriefinfo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
