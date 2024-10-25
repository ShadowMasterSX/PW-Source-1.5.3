
#ifndef __GNET_AUTOTEAMPLAYERLEAVE_HPP
#define __GNET_AUTOTEAMPLAYERLEAVE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class AutoTeamPlayerLeave : public GNET::Protocol
{
	#include "autoteamplayerleave"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer::GetInstance()->Send(localsid, this);
	}
};

};

#endif
