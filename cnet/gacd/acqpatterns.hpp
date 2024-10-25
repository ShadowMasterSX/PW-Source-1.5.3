
#ifndef __GNET_ACQPATTERNS_HPP
#define __GNET_ACQPATTERNS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "acstackpattern"

namespace GNET
{

class ACQPatterns : public GNET::Protocol
{
	#include "acqpatterns"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
