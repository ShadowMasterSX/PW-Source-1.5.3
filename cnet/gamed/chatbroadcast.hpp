
#ifndef __GNET_CHATBROADCAST_HPP
#define __GNET_CHATBROADCAST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class ChatBroadCast : public GNET::Protocol
{
	#include "chatbroadcast"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
