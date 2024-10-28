
#ifndef __GNET_MNDOMAINBATTLEENTER_RE_HPP
#define __GNET_MNDOMAINBATTLEENTER_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

bool player_join_mnfaction(int retcode, int role, int64_t faction_id, int world_tag, int domain_id);

namespace GNET
{

class MNDomainBattleEnter_Re : public GNET::Protocol
{
	#include "mndomainbattleenter_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		//if(retcode != ERR_SUCCESS) return;

		player_join_mnfaction(retcode, roleid, unifid, world_tag, domain_id);
	}
};

};

#endif
