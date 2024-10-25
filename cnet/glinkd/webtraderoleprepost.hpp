
#ifndef __GNET_WEBTRADEROLEPREPOST_HPP
#define __GNET_WEBTRADEROLEPREPOST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class WebTradeRolePrePost : public GNET::Protocol
{
	#include "webtraderoleprepost"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		if (!GLinkServer::GetInstance()->ValidUser(sid,userid)) return;
		this->localsid=sid;
		if ( GDeliveryClient::GetInstance()->SendProtocol(this) )
		{
			GLinkServer::GetInstance()->ChangeState(sid,&state_GWebTradeOpReceive);
		}
		else
			GLinkServer::GetInstance()->SessionError(sid,ERR_COMMUNICATION,"Server Network Error.");
	}
};

};

#endif
