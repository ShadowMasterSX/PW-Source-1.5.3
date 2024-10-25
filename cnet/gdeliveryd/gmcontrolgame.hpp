
#ifndef __GNET_GMCONTROLGAME_HPP
#define __GNET_GMCONTROLGAME_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gproviderserver.hpp"


namespace GNET
{

class GMControlGame : public GNET::Protocol
{
	#include "gmcontrolgame"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		GProviderServer::GetInstance()->DeliverGameControl(this, sid);
	}

};

};

#endif
