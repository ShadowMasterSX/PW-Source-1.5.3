
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
		Log::formatlog("battle", "register=server:map=%d:serverid=%d:worldtag=%d:sid=%d",map_type,server_id,world_tag,sid);
		BattleManager::GetInstance()->RegisterServer(server_id, world_tag);
	}
};

};

#endif
