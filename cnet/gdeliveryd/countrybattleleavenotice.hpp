
#ifndef __GNET_COUNTRYBATTLELEAVENOTICE_HPP
#define __GNET_COUNTRYBATTLELEAVENOTICE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class CountryBattleLeaveNotice : public GNET::Protocol
{
	#include "countrybattleleavenotice"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("countrybattleleavenotice, roleid=%d, country_id=%d, major_strength=%d, minor_strength=%d", roleid, country_id, major_strength, minor_strength);
		CountryBattleMan* pMan = CountryBattleMan::GetInstance();
		
		pMan->LeaveBattle(roleid, country_id, major_strength, minor_strength);
	}
};

};

#endif
