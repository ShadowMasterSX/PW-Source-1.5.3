
#ifndef __GNET_GETSOLOCHALLENGERANK_HPP
#define __GNET_GETSOLOCHALLENGERANK_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "solochallengerankdata"
#include "solochallengerank.h"

#include "mapuser.h"
#include "getsolochallengerank_re.hpp"
#include "gdeliveryserver.hpp"


namespace GNET
{

class GetSoloChallengeRank : public GNET::Protocol
{
	#include "getsolochallengerank"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
        Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
        PlayerInfo* pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
        if (pinfo == NULL) return;

        GetSoloChallengeRank_Re re;
        re.retcode = 0;
        re.roleid = roleid;
        re.ranktype = ranktype;
        re.cls = cls;
        re.localsid = pinfo->localsid;

        if (!SoloChallengeRank::GetInstance().GetRankInfo(ranktype, cls, re.next_sort_time, re.data)) re.retcode = -1;
        GDeliveryServer::GetInstance()->Send(pinfo->linksid, re);
	}
};

};

#endif
