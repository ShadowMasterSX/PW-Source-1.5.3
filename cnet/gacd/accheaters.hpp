
#ifndef __GNET_ACCHEATERS_HPP
#define __GNET_ACCHEATERS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class ACCheaters : public GNET::Protocol
{
	#include "accheaters"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
