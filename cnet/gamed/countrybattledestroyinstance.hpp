
#ifndef __GNET_COUNTRYBATTLEDESTROYINSTANCE_HPP
#define __GNET_COUNTRYBATTLEDESTROYINSTANCE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void country_battle_destory_instance(int battleid, int worldtag);

namespace GNET
{

class CountryBattleDestroyInstance : public GNET::Protocol
{
	#include "countrybattledestroyinstance"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		country_battle_destory_instance(domain_id, world_tag);
	}
};

};

#endif
