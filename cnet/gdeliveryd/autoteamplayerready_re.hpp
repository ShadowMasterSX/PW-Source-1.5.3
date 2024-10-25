
#ifndef __GNET_AUTOTEAMPLAYERREADY_RE_HPP
#define __GNET_AUTOTEAMPLAYERREADY_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class AutoTeamPlayerReady_Re : public GNET::Protocol
{
	#include "autoteamplayerready_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		AutoTeamMan::GetInstance()->OnPlayerReady(roleid, leader_id, retcode);	
	}
};

};

#endif
