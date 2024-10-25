
#ifndef __GNET_COUNTRYBATTLESERVERREGISTER_HPP
#define __GNET_COUNTRYBATTLESERVERREGISTER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class CountryBattleServerRegister : public GNET::Protocol
{
	#include "countrybattleserverregister"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Log::formatlog("countrybattle", "register=server:server_type=%d:war_type=%d:serverid=%d:worldtag=%d:sid=%d", server_type, war_type, server_id, world_tag, sid);
		CountryBattleMan::GetInstance()->RegisterServer(server_type, war_type, server_id, world_tag);
	}
};

};

#endif
