
#ifndef __GNET_DOMAINCMD_HPP
#define __GNET_DOMAINCMD_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class DomainCmd : public GNET::Protocol
{
	#include "domaincmd"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		localsid = sid;
		if(cmd.size())
			GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
