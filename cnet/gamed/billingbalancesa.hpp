
#ifndef __GNET_BILLINGBALANCESA_HPP
#define __GNET_BILLINGBALANCESA_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class BillingBalanceSA : public GNET::Protocol
{
	#include "billingbalancesa"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
