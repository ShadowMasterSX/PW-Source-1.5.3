
#ifndef __GNET_ACCONNECTRE_HPP
#define __GNET_ACCONNECTRE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class ACConnectRe : public GNET::Protocol
{
	#include "acconnectre"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
