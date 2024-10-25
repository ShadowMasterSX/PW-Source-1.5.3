
#ifndef __GNET_TANKBATTLESTART_HPP
#define __GNET_TANKBATTLESTART_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class TankBattleStart : public GNET::Protocol
{
	#include "tankbattlestart"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
