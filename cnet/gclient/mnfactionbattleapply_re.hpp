
#ifndef __GNET_MNFACTIONBATTLEAPPLY_RE_HPP
#define __GNET_MNFACTIONBATTLEAPPLY_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class MNFactionBattleApply_Re : public GNET::Protocol
{
	#include "mnfactionbattleapply_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
