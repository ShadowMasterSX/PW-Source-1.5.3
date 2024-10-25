
#ifndef __GNET_QUERYPLAYERSTATUS_HPP
#define __GNET_QUERYPLAYERSTATUS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gproviderserver.hpp"
#include "playerstatusannounce.hpp"
#include "gdeliveryserver.hpp"
#include "mapuser.h"
namespace GNET
{

class QueryPlayerStatus : public GNET::Protocol
{
	#include "queryplayerstatus"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());

		PlayerStatusAnnounce psa_on;
		PlayerStatusAnnounce psa_off;
		int roleid;
		for (size_t i=0;i<rolelist.size();i++)
		{
			roleid=rolelist[i];
			PlayerInfo * pinfo = UserContainer::GetInstance().FindRole((roleid));
			psa_on.status=_STATUS_ONGAME;
			psa_off.status=_STATUS_OFFLINE;
			if (NULL!=pinfo)
			{
				if( pinfo->ingame)
					psa_on.playerstatus_list.add(OnlinePlayerStatus(pinfo->roleid,pinfo->gameid));
				else
					psa_off.playerstatus_list.add(OnlinePlayerStatus(pinfo->roleid,pinfo->gameid));
			}
		}
		if (psa_on.playerstatus_list.size()>0)
			manager->Send(sid,psa_on);
		if (psa_off.playerstatus_list.size()>0)
			manager->Send(sid,psa_off);
	}
};

};

#endif
