
#ifndef __GNET_BILLINGBALANCE_HPP
#define __GNET_BILLINGBALANCE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class BillingBalance : public GNET::Protocol
{
	#include "billingbalance"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
