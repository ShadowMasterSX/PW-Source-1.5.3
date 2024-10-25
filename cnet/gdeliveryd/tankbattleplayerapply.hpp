
#ifndef __GNET_TANKBATTLEPLAYERAPPLY_HPP
#define __GNET_TANKBATTLEPLAYERAPPLY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "tankbattlemanager.h"

namespace GNET
{

class TankBattlePlayerApply : public GNET::Protocol
{
	#include "tankbattleplayerapply"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		TankBattleManager::GetInstance()->PlayerApply(roleid,model);
	}
};

};

#endif
