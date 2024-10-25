
#ifndef __GNET_STATINFOVITAL_HPP
#define __GNET_STATINFOVITAL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class StatInfoVital : public GNET::Protocol
{
	#include "statinfovital"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
