
#ifndef __GNET_PLAYERPROFILEGETPROFILEDATA_HPP
#define __GNET_PLAYERPROFILEGETPROFILEDATA_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class PlayerProfileGetProfileData : public GNET::Protocol
{
	#include "playerprofilegetprofiledata"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(!GLinkServer::ValidRole(sid, roleid)) {
			GLinkServer::GetInstance()->SessionError(sid, ERR_INVALID_ACCOUNT, "Error userid or roleid.");
			return;
		}
		
		DEBUG_PRINT("PlayerProfileGetProfileData: %d\n", roleid);
		localsid = sid;
		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
