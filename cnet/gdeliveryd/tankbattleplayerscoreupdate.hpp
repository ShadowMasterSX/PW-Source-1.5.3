
#ifndef __GNET_TANKBATTLEPLAYERSCOREUPDATE_HPP
#define __GNET_TANKBATTLEPLAYERSCOREUPDATE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "tankbattleplayerscoreinfo"
#include "tankbattlemanager.h"

namespace GNET
{

class TankBattlePlayerScoreUpdate : public GNET::Protocol
{
	#include "tankbattleplayerscoreupdate"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		TankBattleManager::GetInstance()->PlayerScoreUpdate(battle_id,world_tag,player_scores);
	}
};

};

#endif
