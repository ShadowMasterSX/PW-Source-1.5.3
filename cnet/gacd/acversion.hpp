
#ifndef __GNET_ACVERSION_HPP
#define __GNET_ACVERSION_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class ACVersion : public GNET::Protocol
{
	#include "acversion"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
