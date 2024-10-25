
#ifndef __GNET_ANNOUNCEZONEID_HPP
#define __GNET_ANNOUNCEZONEID_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gfactionserver.hpp"
namespace GNET
{

class AnnounceZoneid : public GNET::Protocol
{
	#include "announcezoneid"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("Receive zoneid=%d from delivery\n",zoneid);
		GFactionServer::GetInstance()->zoneid=zoneid;
	}
};

};

#endif
