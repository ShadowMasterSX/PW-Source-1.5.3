
#ifndef __GNET_TRADEADDGOODS_HPP
#define __GNET_TRADEADDGOODS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "groleinventory"
#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
namespace GNET
{

class TradeAddGoods : public GNET::Protocol
{
	#include "tradeaddgoods"

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
