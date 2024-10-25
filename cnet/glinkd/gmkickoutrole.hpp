
#ifndef __GNET_GMKICKOUTROLE_HPP
#define __GNET_GMKICKOUTROLE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
#include "privilege.hxx"

namespace GNET
{

class GMKickoutRole : public GNET::Protocol
{
	#include "gmkickoutrole"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer* lsm=GLinkServer::GetInstance();
		if (! lsm->PrivilegeCheck(sid, gmroleid,Privilege::PRV_FORCE_OFFLINE) )
		{
			Log::log(LOG_ERR,"WARNING: user %d try to use GM_OP_KICKOUT privilege that he doesn't have.\n",gmroleid);
			return;
		}
		char content[256];
		sprintf(content,"KickRole: kickroleid=%d,forbidtime=%d",kickroleid,forbid_time);
		Log::gmoperate(gmroleid,Privilege::PRV_FORCE_OFFLINE,content);
		this->localsid=sid;
		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
