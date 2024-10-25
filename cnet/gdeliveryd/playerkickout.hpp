
#ifndef __GNET_PLAYERKICKOUT_HPP
#define __GNET_PLAYERKICKOUT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class PlayerKickout : public GNET::Protocol
{
	#include "playerkickout"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
