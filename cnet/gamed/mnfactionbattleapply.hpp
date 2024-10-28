
#ifndef __GNET_MNFACTIONBATTLEAPPLY_HPP
#define __GNET_MNFACTIONBATTLEAPPLY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gmailsyncdata"

namespace GNET
{

class MNFactionBattleApply : public GNET::Protocol
{
	#include "mnfactionbattleapply"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
