
#ifndef __GNET_COUNTRYBATTLESYNCPLAYERPOS_HPP
#define __GNET_COUNTRYBATTLESYNCPLAYERPOS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void player_country_territory_move(int role_id, int world_tag, float posx, float posy, float posz, bool capital);

namespace GNET
{

class CountryBattleSyncPlayerPos : public GNET::Protocol
{
	#include "countrybattlesyncplayerpos"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		player_country_territory_move(roleid, worldtag, posx, posy, posz, is_capital);
	}
};

};

#endif
