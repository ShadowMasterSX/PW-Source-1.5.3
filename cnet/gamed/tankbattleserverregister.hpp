
#ifndef __GNET_TANKBATTLESERVERREGISTER_HPP
#define __GNET_TANKBATTLESERVERREGISTER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class TankBattleServerRegister : public GNET::Protocol
{
	#include "tankbattleserverregister"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
