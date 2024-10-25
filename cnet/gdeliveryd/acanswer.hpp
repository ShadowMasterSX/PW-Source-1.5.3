
#ifndef __GNET_ACANSWER_HPP
#define __GNET_ACANSWER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "ganticheatclient.hpp"

namespace GNET
{

class ACAnswer : public GNET::Protocol
{
	#include "acanswer"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		GAntiCheatClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
