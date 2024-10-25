
#ifndef __GNET_COUNTRYBATTLEGETKINGCOMMANDPOINT_HPP
#define __GNET_COUNTRYBATTLEGETKINGCOMMANDPOINT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class CountryBattleGetKingCommandPoint : public GNET::Protocol
{
	#include "countrybattlegetkingcommandpoint"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(!GLinkServer::ValidRole(sid, roleid)) {
			GLinkServer::GetInstance()->SessionError(sid, ERR_INVALID_ACCOUNT, "Error userid or roleid.");
			return;
		}
		
		DEBUG_PRINT("CountryBattleGetKingCommandPoint: %d\n", roleid);
		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
