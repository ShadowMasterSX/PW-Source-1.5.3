
#ifndef __GNET_COUNTRYBATTLESTART_RE_HPP
#define __GNET_COUNTRYBATTLESTART_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class CountryBattleStart_Re : public GNET::Protocol
{
	#include "countrybattlestart_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		LOG_TRACE( "CountryBattleStart_Re: retcode=%d battle_id=%d world_tag=%d defender=%d attacker=%d.\n", retcode, battle_id, world_tag, defender, attacker);
		CountryBattleMan::OnBattleStart(battle_id, world_tag, retcode, defender, attacker);
	}
};

};

#endif
