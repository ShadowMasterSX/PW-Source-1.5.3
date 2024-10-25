
#ifndef __GNET_BATTLEGETMAP_HPP
#define __GNET_BATTLEGETMAP_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "battlegetmap_re.hpp"

namespace GNET
{

class BattleGetMap : public GNET::Protocol
{
	#include "battlegetmap"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if (!GLinkServer::ValidRole(sid,roleid))
		{
			GLinkServer::GetInstance()->SessionError(sid,ERR_INVALID_ACCOUNT,"Error userid or roleid.");
			return;
		}	
		DEBUG_PRINT("BattleGetMap: %d\n", roleid);
		localsid = sid;
		if (!GDeliveryClient::GetInstance()->SendProtocol(this))
			manager->Send(sid,BattleGetMap_Re(ERR_FC_OUTOFSERVICE, 0, 0));
	}
};

};

#endif
