
#ifndef __GNET_COUNTRYBATTLEKINGASSIGNASSAULT_HPP
#define __GNET_COUNTRYBATTLEKINGASSIGNASSAULT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class CountryBattleKingAssignAssault : public GNET::Protocol
{
	#include "countrybattlekingassignassault"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(!GLinkServer::ValidRole(sid, king_roleid)) {
			GLinkServer::GetInstance()->SessionError(sid, ERR_INVALID_ACCOUNT, "Error userid or roleid.");
			return;
		}
		
		DEBUG_PRINT("CountryBattleKingAssignAssault: %d\n", king_roleid);
		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
