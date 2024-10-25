
#ifndef __GNET_BILLINGCANCEL_HPP
#define __GNET_BILLINGCANCEL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class BillingCancel : public GNET::Protocol
{
	#include "billingcancel"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		PlayerInfo * role = UserContainer::GetInstance().FindRoleOnline(userid);
		if(!role)
			return;
		userid = role->userid;
		GAuthClient::GetInstance()->SendProtocol(this);

		DEBUG_PRINT("BillingCancel,userid=%d,chargeno=%.*s",userid,(char *)chargeno.begin());
		
	}
};

};

#endif
