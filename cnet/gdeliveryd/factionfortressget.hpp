
#ifndef __GNET_FACTIONFORTRESSGET_HPP
#define __GNET_FACTIONFORTRESSGET_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "factionfortressget_re.hpp"
#include "factionfortressmanager.h"

namespace GNET
{

class FactionFortressGet : public GNET::Protocol
{
	#include "factionfortressget"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("factionfortressget: receive. roleid=%d factionid=%d",roleid,factionid);
		
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if(NULL != pinfo)
		{
			FactionFortressGet_Re re;
			re.retcode = ERR_SUCCESS;
			re.localsid = pinfo->localsid;
			if(!FactionFortressMan::GetInstance().GetFortress(factionid,re.brief))
				re.retcode = ERR_FF_FORTRESS_NOT_EXIST;
			GDeliveryServer::GetInstance()->Send( pinfo->linksid,re );	
		
		}
	}
};

};

#endif
