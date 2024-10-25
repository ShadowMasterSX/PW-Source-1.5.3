
#ifndef __GNET_COUNTRYBATTLESTART_HPP
#define __GNET_COUNTRYBATTLESTART_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

bool country_battle_start(int battle_id, int attacker, int defender, int player_limit, int end_time, int attacker_total, int defender_total, int max_total);

namespace GNET
{

class CountryBattleStart : public GNET::Protocol
{
	#include "countrybattlestart"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		country_battle_start(battle_id, attacker, defender, player_limit, end_time, attacker_player_cnt, defender_player_cnt, country_max_player_cnt);
	}
};

};

#endif
