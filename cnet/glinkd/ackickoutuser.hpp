
#ifndef __GNET_ACKICKOUTUSER_HPP
#define __GNET_ACKICKOUTUSER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class ACKickoutUser : public GNET::Protocol
{
	#include "ackickoutuser"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
