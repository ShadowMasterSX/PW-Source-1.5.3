
#ifndef __GNET_PLAYERGIVEPRESENT_HPP
#define __GNET_PLAYERGIVEPRESENT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "groleinventory"

namespace GNET
{

class PlayerGivePresent : public GNET::Protocol
{
	#include "playergivepresent"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
