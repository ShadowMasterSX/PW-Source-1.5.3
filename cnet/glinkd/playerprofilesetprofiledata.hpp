
#ifndef __GNET_PLAYERPROFILESETPROFILEDATA_HPP
#define __GNET_PLAYERPROFILESETPROFILEDATA_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "playerprofiledata"

namespace GNET
{

class PlayerProfileSetProfileData : public GNET::Protocol
{
	#include "playerprofilesetprofiledata"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(!GLinkServer::ValidRole(sid, roleid)) {
			GLinkServer::GetInstance()->SessionError(sid, ERR_INVALID_ACCOUNT, "Error userid or roleid.");
			return;
		}
		
		DEBUG_PRINT("PlayerProfileSetProfileData: %d\n", roleid);
		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
