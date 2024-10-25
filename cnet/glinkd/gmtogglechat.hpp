
#ifndef __GNET_GMTOGGLECHAT_HPP
#define __GNET_GMTOGGLECHAT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
#include "statusannounce.hpp"
#include "privilege.hxx"

namespace GNET
{

class GMToggleChat : public GNET::Protocol
{
	#include "gmtogglechat"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer* lsm=GLinkServer::GetInstance();
		if (! lsm->PrivilegeCheck(sid, gmroleid,Privilege::PRV_CHAT_ORNOT) )
		{
			Log::log(LOG_ERR,"WARNING: user %d try to use PRV_CHAT_ORNOT privilege that he doesn't have.\n",gmroleid);
			return;
		}
		char content[256];
		sprintf(content,"Toggle Chat Or Not");
		Log::gmoperate(gmroleid,Privilege::PRV_CHAT_ORNOT,content);
		//send to deliveryserver
		this->localsid=sid;
		GDeliveryClient::GetInstance()->SendProtocol(this);

	}
};

};

#endif
