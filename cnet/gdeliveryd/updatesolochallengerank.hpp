
#ifndef __GNET_UPDATESOLOCHALLENGERANK_HPP
#define __GNET_UPDATESOLOCHALLENGERANK_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "solochallengerankdata"
#include "solochallengerank.h"

#include "mapuser.h"


namespace GNET
{

class UpdateSoloChallengeRank : public GNET::Protocol
{
	#include "updatesolochallengerank"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
        Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
        PlayerInfo* pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
        if (pinfo == NULL) return;

        SoloChallengeRank::GetInstance().UpdateRankInfo(roleid, pinfo->level, pinfo->cls, pinfo->name, total_time);
	}
};

};

#endif
