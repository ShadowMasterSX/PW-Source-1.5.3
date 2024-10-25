
#ifndef __GNET_BILLINGCONFIRM_RE_HPP
#define __GNET_BILLINGCONFIRM_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "stocklib.h"


namespace GNET
{

class BillingConfirm_Re : public GNET::Protocol
{
	#include "billingconfirm_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(retcode)
			return;
		if(!player_billing_approved(userid,itemid,itemcnt,itemexpire,itemprice))
		{
			SendBillingCancelSA(userid,(char *)chargeno.begin(),chargeno.size());
		}
	}
};

};

#endif
