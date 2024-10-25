
#ifndef __GNET_FACTIONFORTRESSBATTLELIST_HPP
#define __GNET_FACTIONFORTRESSBATTLELIST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "factionfortressbattlelist_re.hpp"
#include "factionfortressmanager.h"

namespace GNET
{

class FactionFortressBattleList : public GNET::Protocol
{
	#include "factionfortressbattlelist"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("factionfortressbattlelist: receive. roleid=%d",roleid);
		
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if(NULL != pinfo)
		{
			FactionFortressBattleList_Re re;
			re.localsid = pinfo->localsid;
			FactionFortressMan::GetInstance().GetBattleList(re.status, re.list);	
			GDeliveryServer::GetInstance()->Send(pinfo->linksid,re);
		}
	}
};

};

#endif
