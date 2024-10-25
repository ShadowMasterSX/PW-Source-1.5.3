
#ifndef __GNET_ACPROTOSTAT_HPP
#define __GNET_ACPROTOSTAT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "ganticheatclient.hpp"
namespace GNET
{

class ACProtoStat : public GNET::Protocol
{
	#include "acprotostat"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		GAntiCheatClient::GetInstance()->SendProtocol( this );
	}
};

};

#endif
