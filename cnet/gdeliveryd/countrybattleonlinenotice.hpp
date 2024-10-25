
#ifndef __GNET_COUNTRYBATTLEONLINENOTICE_HPP
#define __GNET_COUNTRYBATTLEONLINENOTICE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class CountryBattleOnlineNotice : public GNET::Protocol
{
	#include "countrybattleonlinenotice"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("countrybattleonlinenotice, roleid=%d, country_id=%d, worldtag=%d, minor_str=%d, is_king=%d", roleid, country_id, worldtag, minor_strength, is_king);
		CountryBattleMan* pMan = CountryBattleMan::GetInstance();
		
		pMan->OnPlayerLogin(roleid, country_id, worldtag, minor_strength, is_king);
	}
};

};

#endif
