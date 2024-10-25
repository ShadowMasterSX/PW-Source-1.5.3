
#ifndef __GNET_ACPERIODINFO_HPP
#define __GNET_ACPERIODINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class ACPeriodInfo : public GNET::Protocol
{
	#include "acperiodinfo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
