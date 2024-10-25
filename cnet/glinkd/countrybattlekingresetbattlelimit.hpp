
#ifndef __GNET_COUNTRYBATTLEKINGRESETBATTLELIMIT_HPP
#define __GNET_COUNTRYBATTLEKINGRESETBATTLELIMIT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gcountrybattlelimit"

namespace GNET
{

class CountryBattleKingResetBattleLimit : public GNET::Protocol
{
	#include "countrybattlekingresetbattlelimit"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(!GLinkServer::ValidRole(sid, king_roleid)) {
			GLinkServer::GetInstance()->SessionError(sid, ERR_INVALID_ACCOUNT, "Error userid or roleid.");
			return;
		}
		
		DEBUG_PRINT("CountryBattleKingResetBattleLimit: %d\n", king_roleid);
		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
