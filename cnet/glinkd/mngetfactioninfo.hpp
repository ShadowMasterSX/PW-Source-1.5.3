
#ifndef __GNET_MNGETFACTIONINFO_HPP
#define __GNET_MNGETFACTIONINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class MNGetFactionInfo : public GNET::Protocol
{
	#include "mngetfactioninfo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Log::log(LOG_DEBUG,"file=%s\tfunc=%s\tline=%d\troleid=%d\tunifid=%lld\n", __FILE__, __FUNCTION__, __LINE__, roleid, unifid);
		if (!GLinkServer::ValidRole(sid, roleid)) 
		{
			GLinkServer::GetInstance()->SessionError(sid, ERR_INVALID_ACCOUNT, "Error userid or roleid.");
			return;
		}

		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
