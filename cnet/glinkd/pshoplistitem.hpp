#ifndef __GNET_PSHOPLISTITEM_HPP
#define __GNET_PSHOPLISTITEM_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"

namespace GNET
{

class PShopListItem : public GNET::Protocol
{
	#include "pshoplistitem"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if (!GLinkServer::ValidRole(sid,roleid))
		{
			GLinkServer::GetInstance()->SessionError(sid,ERR_INVALID_ACCOUNT,"Error userid or roleid.");
			return;
		}	
		this->localsid=sid;
		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
