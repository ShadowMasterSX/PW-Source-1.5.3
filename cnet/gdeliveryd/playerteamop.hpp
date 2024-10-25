
#ifndef __GNET_PLAYERTEAMOP_HPP
#define __GNET_PLAYERTEAMOP_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "teambean"
#include "autoteamman.h"

namespace GNET
{

class PlayerTeamOp : public GNET::Protocol
{
	#include "playerteamop"
	
	enum
	{
		TEAM_CREATE = 0,
		TEAM_DISMISS = 1
	};

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(operation == TEAM_CREATE)
		{
			GameTalkManager::GetInstance()->NotifyTeamCreate(team_uid,captain,members);
			AutoTeamMan::GetInstance()->OnPlayerJoinTeam(captain);
			for(unsigned int i = 0; i < members.size(); ++i)
			{
				AutoTeamMan::GetInstance()->OnPlayerJoinTeam(members[i]);
			}
		}
		else if(operation == TEAM_DISMISS)
		{
			GameTalkManager::GetInstance()->NotifyTeamDismiss(team_uid);
		}
	}
};

};

#endif
