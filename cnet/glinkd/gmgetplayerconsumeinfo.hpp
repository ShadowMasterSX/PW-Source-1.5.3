
#ifndef __GNET_GMGETPLAYERCONSUMEINFO_HPP
#define __GNET_GMGETPLAYERCONSUMEINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class GMGetPlayerConsumeInfo : public GNET::Protocol
{
	#include "gmgetplayerconsumeinfo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer* lsm=GLinkServer::GetInstance();
		//valid gmroleid
		if (! lsm->PrivilegeCheck(sid, gmroleid,Privilege::PRV_FORCE_OFFLINE) )
		{
			Log::log(LOG_ERR,"WARNING: user %d try to use PRV_FORCE_OFFLINE privilege that he doesn't have.\n",gmroleid);
			return;
		}
		if (playerlist.size() > 128)
		{
			return;
		}
		//send to delivery	
		localsid=sid;
		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
