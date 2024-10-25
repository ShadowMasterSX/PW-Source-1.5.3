
#ifndef __GNET_TRADESTART_HPP
#define __GNET_TRADESTART_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "glinkserver.hpp"
#include "gdeliveryclient.hpp"

namespace GNET
{

class TradeStart : public GNET::Protocol
{
	#include "tradestart"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GDeliveryClient* dsm=GDeliveryClient::GetInstance();
		GLinkServer* lsm=GLinkServer::GetInstance();
		if (manager == lsm)
		{
			if (!GLinkServer::ValidRole(sid,roleid))
			{
				GLinkServer::GetInstance()->SessionError(sid,ERR_INVALID_ACCOUNT,"Error userid or roleid.");
				return;
			}	
			this->localsid=sid;
			GDeliveryClient::GetInstance()->SendProtocol(this);		
		}
		else if (manager == dsm)
		{
			if (!GLinkServer::ValidRole(localsid,roleid))
				return;
			unsigned int tmplocalsid=localsid;
			this->localsid=_SID_INVALID;
			GLinkServer::GetInstance()->Send(tmplocalsid,this);
		}
	}
};

};

#endif
