
#ifndef __GNET_EXCHANGECONSUMEPOINTS_HPP
#define __GNET_EXCHANGECONSUMEPOINTS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class ExchangeConsumePoints : public GNET::Protocol
{
	#include "exchangeconsumepoints"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		PlayerInfo *pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if (pinfo == NULL) return;

		LOG_TRACE("exchangeconsumepoints roleid %d rewardtype %d", roleid, rewardtype);
		ExchangeConsumePoints_Re re(ERR_SUCCESS, roleid, 0 ,localsid);
		re.retcode = RewardManager::GetInstance()->ExchangePoints(pinfo->userid, rewardtype, re.bonus_add);
		manager->Send(sid, re);
	}
};

};

#endif
