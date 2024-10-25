
#ifndef __GNET_WEBTRADEGETDETAIL_HPP
#define __GNET_WEBTRADEGETDETAIL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "webtrademarket.h"
#include "webtradegetdetail_re.hpp"

namespace GNET
{

class WebTradeGetDetail : public GNET::Protocol
{
	#include "webtradegetdetail"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("WebTradegetdetail: receive. roleid=%d,localsid=%d,sn=%lld\n",
				roleid,localsid,sn);
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(roleid);
		if ( NULL!=pinfo )
		{
			WebTradeGetDetail_Re re;
			re.retcode = ERR_SUCCESS;
			re.sn = sn;
			re.localsid = localsid;
			if ( !WebTradeMarket::GetInstance().GetWebTrade(sn,re.detail) )
				re.retcode = ERR_WT_ENTRY_NOT_FOUND;
			//未与平台同步完毕的寄售角色由于数据量过大，不再发给客户端
			if(re.detail.info.posttype == 4 && re.detail.rolebrief.size() > 256)
				re.detail.rolebrief.clear();	
			GDeliveryServer::GetInstance()->Send( pinfo->linksid,re );	
		}
	}
};

};

#endif
