
#ifndef __GNET_COUNTRYBATTLEGETSCORE_HPP
#define __GNET_COUNTRYBATTLEGETSCORE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class CountryBattleGetScore : public GNET::Protocol
{
	#include "countrybattlegetscore"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if (!GLinkServer::ValidRole(sid, roleid)) {
			GLinkServer::GetInstance()->SessionError(sid, ERR_INVALID_ACCOUNT, "Error userid or roleid.");
			return;
		}
		
		DEBUG_PRINT("CountryBattleGetScore: %d\n", roleid);
		localsid = sid;
		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
