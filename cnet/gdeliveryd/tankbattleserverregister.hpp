
#ifndef __GNET_TANKBATTLESERVERREGISTER_HPP
#define __GNET_TANKBATTLESERVERREGISTER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "tankbattlemanager.h"

namespace GNET
{

class TankBattleServerRegister : public GNET::Protocol
{
	#include "tankbattleserverregister"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		TankBattleManager::GetInstance()->RegisterServerInfo(world_tag,server_id);
	}
};

};

#endif
