
#ifndef __GNET_SENDAUMAIL_HPP
#define __GNET_SENDAUMAIL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class SendAUMail : public GNET::Protocol
{
	#include "sendaumail"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(!GLinkServer::ValidRole(sid,roleid))
			return;
		localsid = sid;
		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
