
#ifndef __GNET_SETUICONFIG_HPP
#define __GNET_SETUICONFIG_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
namespace GNET
{

class SetUIConfig : public GNET::Protocol
{
	#include "setuiconfig"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("glinkd::setuiconfig received roleid=%d, sid=%d\n",roleid,sid);
		if (!GLinkServer::ValidRole(sid,roleid))
		{
			DEBUG_PRINT("glinkd::setuiconfig received roleid=%d, sid=%d. !!! valid user failed. !!! \n",roleid,sid);
			GLinkServer::GetInstance()->SessionError(sid,ERR_INVALID_ACCOUNT,"Error userid or roleid.");
			return;
		}	
		this->localsid=sid;
		GDeliveryClient::GetInstance()->SendProtocol(this);		
	}
};

};

#endif
