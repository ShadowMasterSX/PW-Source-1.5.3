
#ifndef __GNET_COUNTRYBATTLEMOVE_HPP
#define __GNET_COUNTRYBATTLEMOVE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class CountryBattleMove : public GNET::Protocol
{
	#include "countrybattlemove"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(!GLinkServer::ValidRole(sid,roleid)) {
			GLinkServer::GetInstance()->SessionError(sid, ERR_INVALID_ACCOUNT, "Error userid or roleid.");
			return;
		}
		
		DEBUG_PRINT("CountryBattleMove: %d %d\n", roleid, dest);
		localsid = sid;
		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
