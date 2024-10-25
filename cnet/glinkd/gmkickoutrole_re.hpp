
#ifndef __GNET_GMKICKOUTROLE_RE_HPP
#define __GNET_GMKICKOUTROLE_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "statusannounce.hpp"

namespace GNET
{

class GMKickoutRole_Re : public GNET::Protocol
{
	#include "gmkickoutrole_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer::GetInstance()->Send(localsid,this);

	}
};

};

#endif
