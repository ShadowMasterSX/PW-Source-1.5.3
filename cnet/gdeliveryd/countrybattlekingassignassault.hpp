
#ifndef __GNET_COUNTRYBATTLEKINGASSIGNASSAULT_HPP
#define __GNET_COUNTRYBATTLEKINGASSIGNASSAULT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "countrybattleman.h"

namespace GNET
{

class CountryBattleKingAssignAssault : public GNET::Protocol
{
	#include "countrybattlekingassignassault"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		CountryBattleMan::GetInstance()->KingAssignAssault(king_roleid, domain_id, assault_type);
	}
};

};

#endif
