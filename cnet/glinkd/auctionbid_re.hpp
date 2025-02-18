
#ifndef __GNET_AUCTIONBID_RE_HPP
#define __GNET_AUCTIONBID_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "glinkserver.hpp"
#include "gauctionitem"
namespace GNET
{

class AuctionBid_Re : public GNET::Protocol
{
	#include "auctionbid_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer::GetInstance()->Send(localsid,this);
	}
};

};

#endif
