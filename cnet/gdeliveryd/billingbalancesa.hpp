
#ifndef __GNET_BILLINGBALANCESA_HPP
#define __GNET_BILLINGBALANCESA_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gauthclient.hpp"

namespace GNET
{

class BillingBalanceSA : public GNET::Protocol
{
	#include "billingbalancesa"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("BillingBalanceSA,roleid=%d",userid);
		PlayerInfo *role = UserContainer::GetInstance().FindRoleOnline(userid);
		if(!role)
			return;
		userid = role->userid;
		GAuthClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
