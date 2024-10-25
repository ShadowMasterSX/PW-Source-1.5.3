
#ifndef __GNET_BILLINGCONFIRM_HPP
#define __GNET_BILLINGCONFIRM_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class BillingConfirm : public GNET::Protocol
{
	#include "billingconfirm"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
	}
};

};

#endif
