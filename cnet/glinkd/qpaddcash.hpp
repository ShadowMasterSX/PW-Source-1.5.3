
#ifndef __GNET_QPADDCASH_HPP
#define __GNET_QPADDCASH_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryclient.hpp"
#include "qpaddcash_re.hpp"

namespace GNET
{

class QPAddCash : public GNET::Protocol
{
	#include "qpaddcash"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if (!GLinkServer::ValidUser(sid,userid)) 
		{
			return;
		}

		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
