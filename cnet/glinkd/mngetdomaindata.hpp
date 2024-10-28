
#ifndef __GNET_MNGETDOMAINDATA_HPP
#define __GNET_MNGETDOMAINDATA_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class MNGetDomainData : public GNET::Protocol
{
	#include "mngetdomaindata"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Log::log(LOG_DEBUG,"file=%s\tfunc=%s\tline=%d\troleid=%d\n", __FILE__, __FUNCTION__, __LINE__, roleid);
		if (!GLinkServer::ValidRole(sid, roleid)) 
		{
			GLinkServer::GetInstance()->SessionError(sid, ERR_INVALID_ACCOUNT, "Error userid or roleid.");
			return;
		}

		localsid = sid;
		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
