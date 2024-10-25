
#ifndef __GNET_COUNTRYBATTLEENTER_HPP
#define __GNET_COUNTRYBATTLEENTER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void player_enter_country_battle(int role_id, int world_tag, int battle_id);

namespace GNET
{

class CountryBattleEnter : public GNET::Protocol
{
	#include "countrybattleenter"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		player_enter_country_battle(roleid,world_tag,battle_id);
	}
};

};

#endif
