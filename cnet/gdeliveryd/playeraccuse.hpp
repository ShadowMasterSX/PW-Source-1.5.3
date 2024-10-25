
#ifndef __GNET_PLAYERACCUSE_HPP
#define __GNET_PLAYERACCUSE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "playeraccuse_re.hpp"
#include "acaccuse.hpp"
#include "ganticheatclient.hpp"

namespace GNET
{

class PlayerAccuse : public GNET::Protocol
{
	#include "playeraccuse"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfosrc = UserContainer::GetInstance().FindRoleOnline(roleid);
		PlayerInfo * pinfodst = UserContainer::GetInstance().FindRoleOnline(dst_roleid);

		if(pinfosrc)
		{
			if(pinfodst && content.size())	
			{
				ACAccuse proto(GDeliveryServer::GetInstance()->GetZoneid(), pinfosrc->roleid, 
						pinfosrc->userid, pinfodst->roleid, pinfodst->userid, content);
			
				if(!GAntiCheatClient::GetInstance()->SendProtocol(proto))
				{
					Log::log(LOG_ERR,"gdelveryd:playeraccuse: no ac connect , roleid=%d,destid=%d",	roleid, dst_roleid);		
				}
			}
			else
			{
				PlayerAccuse_Re proto(pinfosrc->localsid, dst_roleid, 4);
				GDeliveryServer::GetInstance()->Send(sid,proto);
			}
		}
	}
};

};

#endif
