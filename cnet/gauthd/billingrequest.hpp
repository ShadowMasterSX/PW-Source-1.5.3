
#ifndef __GNET_BILLINGREQUEST_HPP
#define __GNET_BILLINGREQUEST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class BillingRequest : public GNET::Protocol
{
	#include "billingrequest"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
