
#ifndef __GNET_FACTIONRESOURCEBATTLEGETRECORD_HPP
#define __GNET_FACTIONRESOURCEBATTLEGETRECORD_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class FactionResourceBattleGetRecord : public GNET::Protocol
{
	#include "factionresourcebattlegetrecord"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if (!GLinkServer::ValidRole(sid, roleid)) {
			GLinkServer::GetInstance()->SessionError(sid, ERR_INVALID_ACCOUNT, "Error userid or roleid.");
			return;
		}
		
		DEBUG_PRINT("FactionResourceBattleGetRecord: %d\n", roleid);
		localsid = sid;
		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
