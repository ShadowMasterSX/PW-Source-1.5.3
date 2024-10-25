
#ifndef __GNET_BATTLESERVERREGISTER_HPP
#define __GNET_BATTLESERVERREGISTER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class BattleServerRegister : public GNET::Protocol
{
	#include "battleserverregister"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
