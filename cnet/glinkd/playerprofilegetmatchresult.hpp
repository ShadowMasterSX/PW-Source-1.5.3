
#ifndef __GNET_PLAYERPROFILEGETMATCHRESULT_HPP
#define __GNET_PLAYERPROFILEGETMATCHRESULT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class PlayerProfileGetMatchResult : public GNET::Protocol
{
	#include "playerprofilegetmatchresult"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(!GLinkServer::ValidRole(sid, roleid)) {
			GLinkServer::GetInstance()->SessionError(sid, ERR_INVALID_ACCOUNT, "Error userid or roleid.");
			return;
		}
		
		DEBUG_PRINT("PlayerProfileGetMatchResult: %d\n", roleid);
		localsid = sid;
		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
