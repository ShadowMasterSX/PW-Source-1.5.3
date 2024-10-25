
#ifndef __GNET_BILLINGREQUEST_HPP
#define __GNET_BILLINGREQUEST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "stocklib.h"
#include <glog.h>

bool player_billing_approved(int roleid, int itemid, int itemnum,int expire_time, int cost);
namespace GNET
{

class BillingRequest : public GNET::Protocol
{
	#include "billingrequest"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(result)
			return;
		if(request==210)
			return;
		if(request==100)
		{
			SendBillingConfirm(userid, itemid, itemnum, timeout, amount, (const char *)bxtxno.begin(), bxtxno.size());
		}
		else if(!player_billing_approved(userid, itemid, itemnum, timeout,amount))
		{
			SendBillingCancel(userid, itemid, itemnum, timeout, amount, (const char *)bxtxno.begin(), bxtxno.size());
		}
		else
		{
			GLog::formatlog("formatlog:billing:userid=%d:itemid=%d:itemnum=%d:timeout=%d:amount=%d:bxtxno=%.*s:menuid=%.*s",
				userid, itemid, itemnum, timeout, amount, bxtxno.size(), (char*)bxtxno.begin(), 
				menuid.size(), (char*)menuid.begin());
		}
	}
};

};

#endif

