
#ifndef __GNET_ACCOUNTINGRESPONSE_HPP
#define __GNET_ACCOUNTINGRESPONSE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gauthclient.hpp"
#include "security.h"
namespace GNET
{

class AccountingResponse : public GNET::Protocol
{
	#include "accountingresponse"
	void Process(Manager *manager, Manager::Session::ID sid)
	{
	}
};

};

#endif
