
#ifndef __GNET_PLAYERENTERLEAVEGT_HPP
#define __GNET_PLAYERENTERLEAVEGT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class PlayerEnterLeaveGT : public GNET::Protocol
{
	#include "playerenterleavegt"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
