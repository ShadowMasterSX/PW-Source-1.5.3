
#ifndef __GNET_PLAYERGIVEPRESENT_RE_HPP
#define __GNET_PLAYERGIVEPRESENT_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"

namespace GNET
{

class PlayerGivePresent_Re : public GNET::Protocol
{
	#include "playergivepresent_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer * lsm = GLinkServer::GetInstance();
		int lsid = localsid;
		localsid = 0;
		lsm->Send(lsid, this);
	}
};

};

#endif
