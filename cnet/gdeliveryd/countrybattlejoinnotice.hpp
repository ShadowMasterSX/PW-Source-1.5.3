
#ifndef __GNET_COUNTRYBATTLEJOINNOTICE_HPP
#define __GNET_COUNTRYBATTLEJOINNOTICE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class CountryBattleJoinNotice : public GNET::Protocol
{
	#include "countrybattlejoinnotice"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("countrybattlejoinnotice, roleid=%d, country_id=%d, worldtag=%d, major_strength=%d, minor_strength=%d, is_king=%d", 
			roleid, country_id, worldtag, major_strength, minor_strength, is_king);
		
		CountryBattleMan::OnPlayerJoinBattle(roleid, country_id, worldtag, major_strength, minor_strength, is_king);
	}
};

};

#endif
