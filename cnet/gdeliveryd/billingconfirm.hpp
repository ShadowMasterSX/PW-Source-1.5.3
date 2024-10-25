
#ifndef __GNET_BILLINGCONFIRM_HPP
#define __GNET_BILLINGCONFIRM_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gauthclient.hpp"
#include "conv_charset.h"
namespace GNET
{

class BillingConfirm : public GNET::Protocol
{
	#include "billingconfirm"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		PlayerInfo * role = UserContainer::GetInstance().FindRoleOnline(userid);
		if(!role)
			return;
		userid = role->userid;
		loginip = role->user->ip;
		//Octets name_gbk;
		//CharsetConverter::conv_charset_u2g(itemname,name_gbk);	
		//itemname.swap(name_gbk);

		GAuthClient::GetInstance()->SendProtocol(this);

		DEBUG_PRINT("BillingConfirm, userid=%d,itemid=%d,itemcnt=%d,itemexpire=%d,itemprice=%d,loginip=%d",userid,itemid,itemcnt,itemexpire,itemprice,loginip); 
		
	}
};

};

#endif
