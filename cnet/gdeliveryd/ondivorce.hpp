
#ifndef __GNET_ONDIVORCE_HPP
#define __GNET_ONDIVORCE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class OnDivorce : public GNET::Protocol
{
	#include "ondivorce"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
