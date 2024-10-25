
#ifndef __GNET_SENDAUCTIONBID_HPP
#define __GNET_SENDAUCTIONBID_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class SendAuctionBid : public GNET::Protocol
{
	#include "sendauctionbid"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
