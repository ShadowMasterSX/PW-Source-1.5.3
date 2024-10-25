
#ifndef __GNET_FACTIONRESOURCEBATTLEEVENTNOTICE_HPP
#define __GNET_FACTIONRESOURCEBATTLEEVENTNOTICE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class FactionResourceBattleEventNotice : public GNET::Protocol
{
	#include "factionresourcebattleeventnotice"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		FactionResourceBattleMan::GetInstance()->UpdateEvent(event_type, src_faction, dest_faction, domain_id, leader_role, members);
	}
};

};

#endif
