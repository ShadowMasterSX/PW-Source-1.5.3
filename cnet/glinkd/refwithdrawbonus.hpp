
#ifndef __GNET_REFWITHDRAWBONUS_HPP
#define __GNET_REFWITHDRAWBONUS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class RefWithdrawBonus : public GNET::Protocol
{
	#include "refwithdrawbonus"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if (!GLinkServer::ValidRole(sid, roleid))
			return; 
		localsid = sid;
		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
