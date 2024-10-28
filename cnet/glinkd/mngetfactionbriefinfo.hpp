
#ifndef __GNET_MNGETFACTIONBRIEFINFO_HPP
#define __GNET_MNGETFACTIONBRIEFINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class MNGetFactionBriefInfo : public GNET::Protocol
{
	#include "mngetfactionbriefinfo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Log::log(LOG_DEBUG, "file=%s\tfunc=%s\tline=%d\troleid=%d\tfid=%d\tzoneid=%d\n", __FILE__, __FUNCTION__, __LINE__, roleid, fid, zoneid);
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
