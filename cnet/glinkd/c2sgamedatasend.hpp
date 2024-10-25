
#ifndef __GNET_C2SGAMEDATASEND_HPP
#define __GNET_C2SGAMEDATASEND_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class C2SGamedataSend : public GNET::Protocol
{
	#include "c2sgamedatasend"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
