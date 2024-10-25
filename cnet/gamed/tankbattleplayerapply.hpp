
#ifndef __GNET_TANKBATTLEPLAYERAPPLY_HPP
#define __GNET_TANKBATTLEPLAYERAPPLY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class TankBattlePlayerApply : public GNET::Protocol
{
	#include "tankbattleplayerapply"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
