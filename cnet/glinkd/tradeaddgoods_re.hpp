
#ifndef __GNET_TRADEADDGOODS_RE_HPP
#define __GNET_TRADEADDGOODS_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "groleinventory"
#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"
namespace GNET
{

class TradeAddGoods_Re : public GNET::Protocol
{
	#include "tradeaddgoods_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer::GetInstance()->Send(localsid,this);
	}
};

};

#endif
