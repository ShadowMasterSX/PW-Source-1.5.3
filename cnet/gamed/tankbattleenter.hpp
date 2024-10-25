
#ifndef __GNET_TANKBATTLEENTER_HPP
#define __GNET_TANKBATTLEENTER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void player_enter_trick_battle(int role, int world_tag, int battle_id, int chariot);

namespace GNET
{

class TankBattleEnter : public GNET::Protocol
{
	#include "tankbattleenter"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		player_enter_trick_battle(roleid, world_tag, battle_id, model);
	}
};

};

#endif
