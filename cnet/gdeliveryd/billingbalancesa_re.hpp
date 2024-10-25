
#ifndef __GNET_BILLINGBALANCESA_RE_HPP
#define __GNET_BILLINGBALANCESA_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class BillingBalanceSA_Re : public GNET::Protocol
{
	#include "billingbalancesa_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("BillingBalanceSA_Re, retcode=%d,userid=%d,cashremain=%d",retcode,userid,cashremain);
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		UserInfo * pinfo = UserContainer::GetInstance().FindUser(userid);
		if ( NULL!=pinfo && pinfo->status==_STATUS_ONGAME )
		{
			userid = pinfo->roleid;
			GProviderServer::GetInstance()->DispatchProtocol(pinfo->gameid, this);
		}
	}
};

};

#endif
