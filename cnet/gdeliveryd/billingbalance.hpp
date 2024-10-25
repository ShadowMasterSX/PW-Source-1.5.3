
#ifndef __GNET_BILLINGBALANCE_HPP
#define __GNET_BILLINGBALANCE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gauthclient.hpp"

namespace GNET
{

class BillingBalance : public GNET::Protocol
{
	#include "billingbalance"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("BillingBalance, userid=%d,result=%d,balance=%d", userid, result, balance);
		if(manager==GAuthClient::GetInstance())
		{
			Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
			UserInfo * pinfo = UserContainer::GetInstance().FindUser(userid);
			if ( NULL!=pinfo && pinfo->status==_STATUS_ONGAME )
			{
				userid = pinfo->roleid;
				GProviderServer::GetInstance()->DispatchProtocol(pinfo->gameid, this);
			}
		}
		else
		{
			PlayerInfo * role = UserContainer::GetInstance().FindRoleOnline(userid);
			if(!role)
				return;
			userid = role->userid;
			GAuthClient::GetInstance()->SendProtocol(this);
		}
	}
};

};

#endif
