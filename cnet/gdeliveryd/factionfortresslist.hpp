
#ifndef __GNET_FACTIONFORTRESSLIST_HPP
#define __GNET_FACTIONFORTRESSLIST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "factionfortresslist_re.hpp"
#include "factionfortressmanager.h"

namespace GNET
{

class FactionFortressList : public GNET::Protocol
{
	#include "factionfortresslist"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("factionfortresslist: receive. roleid=%d,begin=%d",roleid,begin);
		
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if(NULL != pinfo)
		{
			FactionFortressList_Re re;
			re.begin = begin;
			re.localsid = pinfo->localsid;
			FactionFortressMan::GetInstance().GetFortressList(re.begin, re.status, re.list);	
			GDeliveryServer::GetInstance()->Send(pinfo->linksid,re);
		}
	}
};

};

#endif
