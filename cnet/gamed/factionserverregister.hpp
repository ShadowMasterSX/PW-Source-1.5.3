
#ifndef __GNET_FACTIONSERVERREGISTER_HPP
#define __GNET_FACTIONSERVERREGISTER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class FactionServerRegister : public GNET::Protocol
{
	#include "factionserverregister"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
