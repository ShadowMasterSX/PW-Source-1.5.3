
#ifndef __GNET_MNDOMAINBATTLESTART_HPP
#define __GNET_MNDOMAINBATTLESTART_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void mnfaction_battle_start(int domain_id, unsigned char domain_type, int64_t owner_faction_id, int64_t attacker_faction_id, int64_t defender_faction_id, int end_timestamp);

namespace GNET
{

class MNDomainBattleStart : public GNET::Protocol
{
	#include "mndomainbattlestart"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		mnfaction_battle_start(domain_id, domain_type, owner, attacker, defender, expiretime);
	}
};

};

#endif
