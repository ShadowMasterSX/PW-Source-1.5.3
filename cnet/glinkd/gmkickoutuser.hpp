
#ifndef __GNET_GMKICKOUTUSER_HPP
#define __GNET_GMKICKOUTUSER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
#include "privilege.hxx"
namespace GNET
{

class GMKickoutUser : public GNET::Protocol
{
	#include "gmkickoutuser"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		//GM封禁帐号，kickuserid实际是被封禁玩家的roleid
		GLinkServer* lsm=GLinkServer::GetInstance();
		if (! lsm->PrivilegeCheck(sid, gmroleid,Privilege::PRV_FORCE_OFFLINE) )
		{
			Log::log(LOG_ERR,"WARNING: user %d try to use GM_OP_KICKOUT privilege that he doesn't have.\n",gmroleid);
			return;
		}
		char content[256];
		sprintf(content,"KickUser: kickuserid=%d,forbidtime=%d",kickuserid,forbid_time);
		Log::gmoperate(gmroleid,Privilege::PRV_FORCE_OFFLINE,content);

		this->localsid=sid;
		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
