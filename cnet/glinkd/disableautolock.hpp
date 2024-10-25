
#ifndef __GNET_DISABLEAUTOLOCK_HPP
#define __GNET_DISABLEAUTOLOCK_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class DisableAutolock : public GNET::Protocol
{
	#include "disableautolock"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
