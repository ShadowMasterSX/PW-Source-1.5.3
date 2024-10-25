
#ifndef __GNET_BATTLESTART_RE_HPP
#define __GNET_BATTLESTART_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class BattleStart_Re : public GNET::Protocol
{
	#include "battlestart_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		LOG_TRACE( "BattleStart_Re: retcode=%d battle_id=%d.\n", retcode, battle_id);
		BattleManager::GetInstance()->OnBattleStart(battle_id, retcode);
	}
};

};

#endif
