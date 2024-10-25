
#ifndef __GNET_BILLINGCONFIRM_RE_HPP
#define __GNET_BILLINGCONFIRM_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class BillingConfirm_Re : public GNET::Protocol
{
	#include "billingconfirm_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		UserInfo * pinfo = UserContainer::GetInstance().FindUser(userid);
		if( NULL != pinfo && pinfo->status == _STATUS_ONGAME )
		{
			userid = pinfo->roleid;
			GProviderServer::GetInstance()->DispatchProtocol(pinfo->gameid, this);
		}
		itemprice *= itemcnt;
		DEBUG_PRINT("BillingConfirm_Re,retcode=%d,userid=%d,itemid=%d,itemcnt=%d,itemexpire=%d,itemprice=%d,cashremain=%d",retcode,userid,itemid,itemcnt,itemexpire,itemprice,cashremain);
	}
};

};

#endif
