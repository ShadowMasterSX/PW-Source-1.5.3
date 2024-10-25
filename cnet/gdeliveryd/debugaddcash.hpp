
#ifndef __GNET_DEBUGADDCASH_HPP
#define __GNET_DEBUGADDCASH_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class DebugAddCash : public GNET::Protocol
{
	#include "debugaddcash"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
