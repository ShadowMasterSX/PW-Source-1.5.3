
#ifndef __GNET_AUCTIONBID_HPP
#define __GNET_AUCTIONBID_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "dbauctionbid.hrp"
#include "gdeliveryserver.hpp"
#include "gamedbclient.hpp"
#include "auctionbid_re.hpp"
#include "auctionmarket.h"
#include "postoffice.h"
#include "mapuser.h"
namespace GNET
{

class AuctionBid : public GNET::Protocol
{
	#include "auctionbid"
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Log::log(LOG_ERR,"gdelivery::auctionbid: obsolete protocol used %d", roleid);
	}
};

};

#endif
