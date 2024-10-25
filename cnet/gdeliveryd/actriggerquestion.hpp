#ifndef __GNET_ACTRIGGERQUESTION_HPP
#define __GNET_ACTRIGGERQUESTION_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "ganticheatclient.hpp"

namespace GNET
{

class ACTriggerQuestion : public GNET::Protocol
{
	#include "actriggerquestion"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		GAntiCheatClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
