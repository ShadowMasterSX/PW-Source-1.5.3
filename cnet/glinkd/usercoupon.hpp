
#ifndef __GNET_USERCOUPON_HPP
#define __GNET_USERCOUPON_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class UserCoupon : public GNET::Protocol
{
	#include "usercoupon"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if (!GLinkServer::GetInstance()->ValidUser(sid,userid)) return;
		this->localsid=sid;
		GDeliveryClient::GetInstance()->SendProtocol(this);
	}
};

};

#endif
