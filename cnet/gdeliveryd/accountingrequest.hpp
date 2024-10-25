
#ifndef __GNET_ACCOUNTINGREQUEST_HPP
#define __GNET_ACCOUNTINGREQUEST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "accntparam"
#include "security.h"
#include "gauthclient.hpp"

namespace GNET
{

class AccountingRequest : public GNET::Protocol
{
	#include "accountingrequest"
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
