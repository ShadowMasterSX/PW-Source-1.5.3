
#ifndef __GNET_GETCNETSERVERCONFIG_HPP
#define __GNET_GETCNETSERVERCONFIG_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class GetCNetServerConfig : public GNET::Protocol
{
	#include "getcnetserverconfig"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(!GLinkServer::ValidRole(sid, roleid)) {
			GLinkServer::GetInstance()->SessionError(sid, ERR_INVALID_ACCOUNT, "Error userid or roleid.");
			return;
		}
		
		DEBUG_PRINT("GetCNetServerConfig: %d\n", roleid);
		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
