
#ifndef __GNET_KICKOUTUSER2_HPP
#define __GNET_KICKOUTUSER2_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class KickoutUser2 : public GNET::Protocol
{
	#include "kickoutuser2"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
