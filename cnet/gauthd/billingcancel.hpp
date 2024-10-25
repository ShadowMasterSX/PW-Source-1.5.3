
#ifndef __GNET_BILLINGCANCEL_HPP
#define __GNET_BILLINGCANCEL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class BillingCancel : public GNET::Protocol
{
	#include "billingcancel"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
