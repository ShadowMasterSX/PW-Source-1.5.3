
#ifndef __GNET_FACTIONFORTRESSENTER_HPP
#define __GNET_FACTIONFORTRESSENTER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "factionfortressmanager.h"
#include "factionfortressenternotice.hpp"

namespace GNET
{

class FactionFortressEnter : public GNET::Protocol
{
	#include "factionfortressenter"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("factionfortressenter: receive. roleid=%d,factionid=%d,dst_factionid=%d",roleid,factionid,dst_factionid);
		
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( roleid );
		if(pinfo != NULL)
		{
			int dst_world_tag = 0;
			if(FactionFortressMan::GetInstance().CheckEnterFortress(factionid,dst_factionid,dst_world_tag))
				manager->Send(sid,FactionFortressEnterNotice(roleid,dst_factionid,dst_world_tag));
		}
	}
};

};

#endif
