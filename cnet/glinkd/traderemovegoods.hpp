
#ifndef __GNET_TRADEREMOVEGOODS_HPP
#define __GNET_TRADEREMOVEGOODS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "groleinventory"
#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
namespace GNET
{

class TradeRemoveGoods : public GNET::Protocol
{
	#include "traderemovegoods"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if (!GLinkServer::ValidRole(sid,roleid))
		{
			GLinkServer::GetInstance()->SessionError(sid,ERR_INVALID_ACCOUNT,"Error userid or roleid.");
			return;
		}	
		this->localsid=sid;
		GDeliveryClient::GetInstance()->SendProtocol(this);		
		DEBUG_PRINT("<<<<<<<<glinkd::receive trade remove goods, send it to delivery.>>>>>>>>>>>\n");
	}
};

};

#endif
