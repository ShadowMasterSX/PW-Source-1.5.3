
#ifndef __GNET_AUCTIONLIST_RE_HPP
#define __GNET_AUCTIONLIST_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "glinkserver.hpp"
#include "gauctionitem"
namespace GNET
{

class AuctionList_Re : public GNET::Protocol
{
	#include "auctionlist_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		GLinkServer::GetInstance()->Send(localsid,this);
	}
};

};

#endif
