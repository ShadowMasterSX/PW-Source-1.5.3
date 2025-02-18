
#ifndef __GNET_AUCTIONATTENDLIST_RE_HPP
#define __GNET_AUCTIONATTENDLIST_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "glinkserver.hpp"
namespace GNET
{

class AuctionAttendList_Re : public GNET::Protocol
{
	#include "auctionattendlist_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer::GetInstance()->Send(localsid,this);
	}
};

};

#endif
