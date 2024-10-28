
#ifndef __GNET_COUNTRYBATTLEAPPLY_HPP
#define __GNET_COUNTRYBATTLEAPPLY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "macros.h"
#include "countrybattleman.h"

namespace GNET
{

class CountryBattleApply : public GNET::Protocol
{
	#include "countrybattleapply"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		CountryBattleMan::OnPlayersApplyBattle(list, sid);	
	}
};

};

#endif
