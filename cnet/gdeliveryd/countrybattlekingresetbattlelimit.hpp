
#ifndef __GNET_COUNTRYBATTLEKINGRESETBATTLELIMIT_HPP
#define __GNET_COUNTRYBATTLEKINGRESETBATTLELIMIT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gcountrybattlelimit"

namespace GNET
{

class CountryBattleKingResetBattleLimit : public GNET::Protocol
{
	#include "countrybattlekingresetbattlelimit"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		CountryBattleMan::GetInstance()->KingResetBattleLimit(king_roleid, domain_id, op, limit);
	}
};

};

#endif
