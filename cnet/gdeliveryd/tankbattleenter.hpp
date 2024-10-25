
#ifndef __GNET_TANKBATTLEENTER_HPP
#define __GNET_TANKBATTLEENTER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class TankBattleEnter : public GNET::Protocol
{
	#include "tankbattleenter"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
