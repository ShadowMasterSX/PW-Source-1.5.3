
#ifndef __GNET_GETREWARDLIST_HPP
#define __GNET_GETREWARDLIST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "rewardmanager.h"
#include "getrewardlist_re.hpp"

namespace GNET
{

class GetRewardList : public GNET::Protocol
{
	#include "getrewardlist"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		PlayerInfo *pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if (pinfo == NULL) return;

		LOG_TRACE("getrewardlist roleid %d start_index %d", roleid, start_index);
		GetRewardList_Re re;
		re.roleid = roleid;
		re.start_index = start_index;
		re.localsid = localsid;
		re.retcode = RewardManager::GetInstance()->GetRewardList(pinfo->userid, re.consume_points, re.total, re.start_index, re.rewardlist);  
		manager->Send(sid, re);
	}
};

};

#endif
