
#ifndef __GNET_PLAYERLOGIN_HPP
#define __GNET_PLAYERLOGIN_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class PlayerLogin : public GNET::Protocol
{
	#include "playerlogin"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
