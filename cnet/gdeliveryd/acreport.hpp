
#ifndef __GNET_ACREPORT_HPP
#define __GNET_ACREPORT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "ganticheatclient.hpp"
namespace GNET
{

class ACReport : public GNET::Protocol
{
	#include "acreport"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		GAntiCheatClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
