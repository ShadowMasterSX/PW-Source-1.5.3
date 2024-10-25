
#ifndef __GNET_QUERYREWARDTYPE_HPP
#define __GNET_QUERYREWARDTYPE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "queryrewardtype_re.hpp"
#include "mappasswd.h"
namespace GNET
{

class QueryRewardType : public GNET::Protocol
{
	#include "queryrewardtype"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline( roleid );
		if(NULL == pinfo)
			return;
		UserInfo* user = pinfo->user;
		int rewardmask = Passwd::GetInstance().GetUserReward(user->userid);
		manager->Send( sid, QueryRewardType_Re( roleid,user->rewardtype, user->rewardtype2, user->rewarddata, rewardmask) );
	}
};

};

#endif
