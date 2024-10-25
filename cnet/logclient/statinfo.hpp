
#ifndef __GNET_STATINFO_HPP
#define __GNET_STATINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class StatInfo : public GNET::Protocol
{
	#include "statinfo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
