
#ifndef __GNET_TRYCHANGEDS_HPP
#define __GNET_TRYCHANGEDS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class TryChangeDS : public GNET::Protocol
{
	#include "trychangeds"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
