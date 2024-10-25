
#ifndef __GNET_DEBUGCOMMAND_HPP
#define __GNET_DEBUGCOMMAND_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class DebugCommand : public GNET::Protocol
{
	#include "debugcommand"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
