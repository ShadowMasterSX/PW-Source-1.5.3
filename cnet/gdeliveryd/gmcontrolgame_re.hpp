
#ifndef __GNET_GMCONTROLGAME_RE_HPP
#define __GNET_GMCONTROLGAME_RE_HPP

#include "rpcdefs.h"
#include "rpc.h"
#include "callid.hxx"
#include "state.hxx"
#include "gproviderserver.hpp"
#include "gmcontrolgameres"

namespace GNET
{

class GMControlGame_Re : public GNET::Protocol
{
	#include "gmcontrolgame_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		GProviderServer::GetInstance()->DeliverGameControlRes(this, sid);
	}
};

};

#endif
