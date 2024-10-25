
#ifndef __GNET_TANKBATTLEEND_HPP
#define __GNET_TANKBATTLEEND_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "tankbattlemanager.h"

namespace GNET
{

class TankBattleEnd : public GNET::Protocol
{
	#include "tankbattleend"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		TankBattleManager::GetInstance()->OnBattleEnd(world_tag,battle_id);
	}
};

};

#endif
