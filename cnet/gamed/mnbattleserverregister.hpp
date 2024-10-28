
#ifndef __GNET_MNBATTLESERVERREGISTER_HPP
#define __GNET_MNBATTLESERVERREGISTER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class MNBattleServerRegister : public GNET::Protocol
{
	#include "mnbattleserverregister"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
