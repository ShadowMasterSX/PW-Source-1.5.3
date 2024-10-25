
#ifndef __GNET_PLAYERTEAMMEMBEROP_HPP
#define __GNET_PLAYERTEAMMEMBEROP_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class PlayerTeamMemberOp : public GNET::Protocol
{
	#include "playerteammemberop"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GameTalkManager::GetInstance()->NotifyTeamMemberOp(team_uid,operation,member);
		if(operation == 0) AutoTeamMan::GetInstance()->OnPlayerJoinTeam(member);
	}
};

};

#endif
