
#ifndef __GNET_COUNTRYBATTLEGETPLAYERLOCATION_HPP
#define __GNET_COUNTRYBATTLEGETPLAYERLOCATION_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class CountryBattleGetPlayerLocation : public GNET::Protocol
{
	#include "countrybattlegetplayerlocation"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if (!GLinkServer::ValidRole(sid, roleid)) {
			GLinkServer::GetInstance()->SessionError(sid, ERR_INVALID_ACCOUNT, "Error userid or roleid.");
			return;
		}
		
		DEBUG_PRINT("CountryBattleGetPlayerLocation: %d\n", roleid);
		localsid = sid;
		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
