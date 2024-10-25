
#ifndef __GNET_TANKBATTLEEND_HPP
#define __GNET_TANKBATTLEEND_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class TankBattleEnd : public GNET::Protocol
{
	#include "tankbattleend"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
