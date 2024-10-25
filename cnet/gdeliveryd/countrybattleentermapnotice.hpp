
#ifndef __GNET_COUNTRYBATTLEENTERMAPNOTICE_HPP
#define __GNET_COUNTRYBATTLEENTERMAPNOTICE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class CountryBattleEnterMapNotice : public GNET::Protocol
{
	#include "countrybattleentermapnotice"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("countrybattleentermapnotice, roleid=%d, worldtag=%d", roleid, worldtag);
		CountryBattleMan* pMan = CountryBattleMan::GetInstance();
		
		pMan->OnPlayerEnterMap(roleid, worldtag);
	}
};

};

#endif
