
#ifndef __GNET_BILLINGBALANCESA_RE_HPP
#define __GNET_BILLINGBALANCESA_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class BillingBalanceSA_Re : public GNET::Protocol
{
	#include "billingbalancesa_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(!retcode)
		{
			player_cash_notify(userid,cashremain);
		}
	}
};

};

#endif
