
#ifndef __GNET_AUMAILSENDED_HPP
#define __GNET_AUMAILSENDED_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class AUMailSended : public GNET::Protocol
{
	#include "aumailsended"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
