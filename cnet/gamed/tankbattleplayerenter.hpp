
#ifndef __GNET_TANKBATTLEPLAYERENTER_HPP
#define __GNET_TANKBATTLEPLAYERENTER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class TankBattlePlayerEnter : public GNET::Protocol
{
	#include "tankbattleplayerenter"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
