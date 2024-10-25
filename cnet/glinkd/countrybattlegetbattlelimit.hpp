
#ifndef __GNET_COUNTRYBATTLEGETBATTLELIMIT_HPP
#define __GNET_COUNTRYBATTLEGETBATTLELIMIT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gcountrybattlelimit"

namespace GNET
{

class CountryBattleGetBattleLimit : public GNET::Protocol
{
	#include "countrybattlegetbattlelimit"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(!GLinkServer::ValidRole(sid, roleid)) {
			GLinkServer::GetInstance()->SessionError(sid, ERR_INVALID_ACCOUNT, "Error userid or roleid.");
			return;
		}
		
		DEBUG_PRINT("CountryBattleGetBattleLimit: %d\n", roleid);
		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
