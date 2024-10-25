
#ifndef __GNET_S2CGAMEDATASEND_HPP
#define __GNET_S2CGAMEDATASEND_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class S2CGamedataSend : public GNET::Protocol
{
	#include "s2cgamedatasend"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
