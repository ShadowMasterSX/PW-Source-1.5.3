
#ifndef __GNET_QUERYREWARDTYPE_RE_HPP
#define __GNET_QUERYREWARDTYPE_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void notify_player_reward(int roleid, int reward, int reward2, int param, int rewardmask);

namespace GNET
{

class QueryRewardType_Re : public GNET::Protocol
{
	#include "queryrewardtype_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		notify_player_reward(roleid,reward,reward2,param,rewardmask);
	}
};

};

#endif
