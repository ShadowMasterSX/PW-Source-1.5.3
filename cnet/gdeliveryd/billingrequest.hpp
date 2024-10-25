
#ifndef __GNET_BILLINGREQUEST_HPP
#define __GNET_BILLINGREQUEST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "billingagent.h"

namespace GNET
{

class BillingRequest : public GNET::Protocol
{
	#include "billingrequest"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(manager==GAuthClient::GetInstance())
		{
			if(!BillingAgent::Instance().Menuid2Itemid(menuid, itemid, itemnum, timeout)) 
				return;
			UserInfo * pinfo = UserContainer::GetInstance().FindUser(userid);
			if ( NULL!=pinfo && pinfo->status==_STATUS_ONGAME )
			{
				userid = pinfo->roleid;
				GProviderServer::GetInstance()->DispatchProtocol(pinfo->gameid, this);
			}
			DEBUG_PRINT("BillingResult, userid=%d,request=%d,result=%d,amount=%d,menuid=%.*s,bxtxno=%.*s", 
				userid, request, result, amount, menuid.size(), (char*)menuid.begin(), bxtxno.size(), 
				(char*)bxtxno.begin());
		}
		else
		{
			if(!BillingAgent::Instance().Itemid2Menuid(itemid, itemnum, timeout, menuid, agtxno)) 
				return;
			PlayerInfo * role = UserContainer::GetInstance().FindRoleOnline(userid);
			if(!role)
				return;
			userid = role->userid;
			GAuthClient::GetInstance()->SendProtocol(this);
			DEBUG_PRINT("BillingRequest, userid=%d,request=%d,itemid=%d,amount=%d,menuid=%.*s,bxtxno=%.*s", 
				userid, request, itemid, amount, menuid.size(), (char*)menuid.begin(), bxtxno.size(), 
				(char*)bxtxno.begin());
		}
	}
};

};

#endif
