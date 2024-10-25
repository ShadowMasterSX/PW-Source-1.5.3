
#ifndef __GNET_FACTIONRESOURCEBATTLEPLAYERQUERY_HPP
#define __GNET_FACTIONRESOURCEBATTLEPLAYERQUERY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class FactionResourceBattlePlayerQuery : public GNET::Protocol
{
	#include "factionresourcebattleplayerquery"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
        FactionResourceBattleMan::GetInstance()->PlayerQueryResult(roleid, faction_id);
	}
};

};

#endif
