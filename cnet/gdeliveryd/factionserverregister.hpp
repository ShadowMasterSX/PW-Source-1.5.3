
#ifndef __GNET_FACTIONSERVERREGISTER_HPP
#define __GNET_FACTIONSERVERREGISTER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "factionfortressmanager.h"

namespace GNET
{

class FactionServerRegister : public GNET::Protocol
{
	#include "factionserverregister"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("factionserverregister: receive. server_id=%d world_tag=%d",server_id,world_tag);
		FactionFortressMan::GetInstance().RegisterServer(server_id, world_tag);
	}
};

};

#endif
