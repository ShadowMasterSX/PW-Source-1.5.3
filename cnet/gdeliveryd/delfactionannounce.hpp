
#ifndef __GNET_DELFACTIONANNOUNCE_HPP
#define __GNET_DELFACTIONANNOUNCE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "battlemanager.h"
#include "factionfortressmanager.h"
#include "removefaction.hpp"
#include "gametalkdefs.h"

namespace GNET
{

class DelFactionAnnounce : public GNET::Protocol
{
	#include "delfactionannounce"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("DelFactionAnnounce: factionid=%d",factionid);
		BattleManager::GetInstance()->OnDelFaction(factionid);
		FactionFortressMan::GetInstance().OnDelFaction(factionid);

		RemoveFaction remove;
		remove.factionid.factionid = (int64_t)factionid;
		remove.factionid.ftype = GT_FACTION_TYPE;
		GameTalkManager::GetInstance()->NotifyUpdate(remove);
	}
};

};

#endif
