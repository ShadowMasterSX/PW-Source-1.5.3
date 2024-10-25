#ifndef __GNET_PSHOPPLAYERGET_HPP
#define __GNET_PSHOPPLAYERGET_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"

namespace GNET
{

class PShopPlayerGet : public GNET::Protocol
{
	#include "pshopplayerget"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(!GLinkServer::ValidRole(sid,roleid))
		{
			GLinkServer::GetInstance()->SessionError(sid,ERR_INVALID_ACCOUNT,"Error userid or roleid.");
			return;
		}	

		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
