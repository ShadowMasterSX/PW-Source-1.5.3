
#ifndef __GNET_MOBILESERVERREGISTER_HPP
#define __GNET_MOBILESERVERREGISTER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class MobileServerRegister : public GNET::Protocol
{
	#include "mobileserverregister"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
