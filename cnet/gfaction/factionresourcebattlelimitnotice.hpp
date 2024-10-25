
#ifndef __GNET_FACTIONRESOURCEBATTLELIMITNOTICE_HPP
#define __GNET_FACTIONRESOURCEBATTLELIMITNOTICE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "../rpcdata/gfactionresourcebattlelimit"

namespace GNET
{

class FactionResourceBattleLimitNotice : public GNET::Protocol
{
	#include "factionresourcebattlelimitnotice"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		Factiondb::GetInstance()->SetPvpStatus(status);
		for(size_t n = 0; n < limit_data.size(); ++n)
		{
			GFactionResourceBattleLimit& limit = limit_data[n];
			Factiondb::GetInstance()->SetPvpMask(limit.faction_id,limit.limit_mask);
		}
	}
};

};

#endif
