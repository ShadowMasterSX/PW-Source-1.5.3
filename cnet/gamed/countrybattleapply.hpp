
#ifndef __GNET_COUNTRYBATTLEAPPLY_HPP
#define __GNET_COUNTRYBATTLEAPPLY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "countrybattleapplyentry"

namespace GNET
{

class CountryBattleApply : public GNET::Protocol
{
	#include "countrybattleapply"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
