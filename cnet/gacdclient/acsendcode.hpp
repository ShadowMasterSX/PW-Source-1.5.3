
#ifndef __GNET_ACSENDCODE_HPP
#define __GNET_ACSENDCODE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class ACSendCode : public GNET::Protocol
{
	#include "acsendcode"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
