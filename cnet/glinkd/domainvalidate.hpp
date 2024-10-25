
#ifndef __GNET_DOMAINVALIDATE_HPP
#define __GNET_DOMAINVALIDATE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class DomainValidate : public GNET::Protocol
{
	#include "domainvalidate"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(manager==GLinkServer::GetInstance())
		{
			localsid = sid;
			GDeliveryClient::GetInstance()->SendProtocol(this);
		}
		else
			GLinkServer::GetInstance()->Send(localsid, this);
	}
};

};

#endif
