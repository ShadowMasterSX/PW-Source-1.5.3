
#ifndef __GNET_GMLISTONLINEUSER_HPP
#define __GNET_GMLISTONLINEUSER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
#include "statusannounce.hpp"
#include "privilege.hxx"
namespace GNET
{

class GMListOnlineUser : public GNET::Protocol
{
	#include "gmlistonlineuser"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer* lsm=GLinkServer::GetInstance();
		if (! lsm->PrivilegeCheck(sid, gmroleid,Privilege::PRV_LISTUSER) )
		{
			Log::log(LOG_ERR,"WARNING: user %d try to use GM_OP_LISTUSER privilege that he doesn't have.\n",gmroleid);
			return;
		}
		char content[256];
		sprintf(content,"ListOnlineUser Infor");
		Log::gmoperate(gmroleid,Privilege::PRV_LISTUSER,content);
		//send to deliveryserver
		this->localsid=sid;
		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
