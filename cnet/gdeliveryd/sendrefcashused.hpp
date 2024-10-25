
#ifndef __GNET_SENDREFCASHUSED_HPP
#define __GNET_SENDREFCASHUSED_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "referencemanager.h"
#include "rewardmanager.h"


namespace GNET
{

class SendRefCashUsed : public GNET::Protocol
{
	#include "sendrefcashused"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		LOG_TRACE("sendrefcashused roleid %d cashused %d level %d", roleid, cash_used, level);
		ReferenceManager::GetInstance()->OnReferralUseCash(roleid, cash_used, level);
		RewardManager::GetInstance()->OnUseCash(roleid, cash_used);
	}
};

};

#endif
