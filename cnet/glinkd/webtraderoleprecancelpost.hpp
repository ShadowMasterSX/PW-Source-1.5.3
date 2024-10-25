
#ifndef __GNET_WEBTRADEROLEPRECANCELPOST_HPP
#define __GNET_WEBTRADEROLEPRECANCELPOST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class WebTradeRolePreCancelPost : public GNET::Protocol
{
	#include "webtraderoleprecancelpost"

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
