
#ifndef __GNET_PLAYERASKFORPRESENT_RE_HPP
#define __GNET_PLAYERASKFORPRESENT_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"

namespace GNET
{

class PlayerAskForPresent_Re : public GNET::Protocol
{
	#include "playeraskforpresent_re"

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
