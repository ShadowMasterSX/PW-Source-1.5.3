
#ifndef __GNET_SENDTASKREWARD_HPP
#define __GNET_SENDTASKREWARD_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "rewardmanager.h"


namespace GNET
{

class SendTaskReward : public GNET::Protocol
{
	#include "sendtaskreward"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		LOG_TRACE("sendtaskreward roleid %d bonus_add %d", roleid, bonus_add);
		RewardManager::GetInstance()->OnTaskReward(roleid, bonus_add);
	}
};

};

#endif
