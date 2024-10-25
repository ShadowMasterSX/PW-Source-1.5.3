
#ifndef __GNET_KICKOUTUSER_HPP
#define __GNET_KICKOUTUSER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class KickoutUser : public GNET::Protocol
{
	#include "kickoutuser"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
