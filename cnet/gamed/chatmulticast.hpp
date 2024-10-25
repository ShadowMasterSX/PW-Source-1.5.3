
#ifndef __GNET_CHATMULTICAST_HPP
#define __GNET_CHATMULTICAST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "player"
namespace GNET
{

class ChatMultiCast : public GNET::Protocol
{
	#include "chatmulticast"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
