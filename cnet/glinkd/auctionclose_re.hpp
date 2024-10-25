
#ifndef __GNET_AUCTIONCLOSE_RE_HPP
#define __GNET_AUCTIONCLOSE_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "glinkserver.hpp"
namespace GNET
{

class AuctionClose_Re : public GNET::Protocol
{
	#include "auctionclose_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		GLinkServer::GetInstance()->Send(localsid,this);
	}
};

};

#endif
