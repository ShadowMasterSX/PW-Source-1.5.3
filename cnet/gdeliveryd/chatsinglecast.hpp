
#ifndef __GNET_CHATSINGLECAST_HPP
#define __GNET_CHATSINGLECAST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class ChatSingleCast : public GNET::Protocol
{
	#include "chatsinglecast"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
