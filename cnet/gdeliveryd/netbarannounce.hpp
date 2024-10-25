
#ifndef __GNET_NETBARANNOUNCE_HPP
#define __GNET_NETBARANNOUNCE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class NetBarAnnounce : public GNET::Protocol
{
	#include "netbarannounce"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
