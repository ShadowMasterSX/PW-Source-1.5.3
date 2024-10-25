
#ifndef __GNET_TANKBATTLEPLAYERLEAVE_HPP
#define __GNET_TANKBATTLEPLAYERLEAVE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class TankBattlePlayerLeave : public GNET::Protocol
{
	#include "tankbattleplayerleave"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
