
#ifndef __GNET_TANKBATTLESTART_HPP
#define __GNET_TANKBATTLESTART_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void trick_battle_start(int battle_id, int player_limit, int end_time);

namespace GNET
{

class TankBattleStart : public GNET::Protocol
{
	#include "tankbattlestart"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		trick_battle_start(battle_id, max_player_cnt, end_time);
	}
};

};

#endif
