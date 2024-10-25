
#ifndef __GNET_DISCONNECTPLAYER_HPP
#define __GNET_DISCONNECTPLAYER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class DisconnectPlayer : public GNET::Protocol
{
	#include "disconnectplayer"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
