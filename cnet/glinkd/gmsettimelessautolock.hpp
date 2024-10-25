
#ifndef __GNET_GMSETTIMELESSAUTOLOCK_HPP
#define __GNET_GMSETTIMELESSAUTOLOCK_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class GMSetTimelessAutoLock : public GNET::Protocol
{
	#include "gmsettimelessautolock"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		GLinkServer* lsm=GLinkServer::GetInstance();
		//valid gmroleid
		if (! lsm->PrivilegeCheck(sid, gmroleid,Privilege::PRV_FORBID_TRADE) )	//暂时使用禁止玩家交易权限
		{
			Log::log(LOG_ERR,"WARNING: user %d try to use PRV_FORCE_OFFLINE privilege that he doesn't have.\n",gmroleid);
			return;
		}
		//send to delivery	
		localsid=sid;
		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
