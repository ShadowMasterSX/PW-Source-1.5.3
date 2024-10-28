
#ifndef __GNET_COUNTRYBATTLEEND_HPP
#define __GNET_COUNTRYBATTLEEND_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gcountrybattlepersonalscore"

namespace GNET
{

class CountryBattleEnd : public GNET::Protocol
{
	#include "countrybattleend"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		CountryBattleMan::OnBattleEnd(battle_id, battle_result, defender, attacker, defender_score, attacker_score);
	}
};

};

#endif
