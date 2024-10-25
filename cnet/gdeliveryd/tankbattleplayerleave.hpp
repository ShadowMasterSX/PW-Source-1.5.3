
#ifndef __GNET_TANKBATTLEPLAYERLEAVE_HPP
#define __GNET_TANKBATTLEPLAYERLEAVE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "tankbattlemanager.h"

namespace GNET
{

class TankBattlePlayerLeave : public GNET::Protocol
{
	#include "tankbattleplayerleave"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		TankBattleManager::GetInstance()->PlayerLeaveBattle(roleid,battle_id,world_tag);
	}
};

};

#endif
