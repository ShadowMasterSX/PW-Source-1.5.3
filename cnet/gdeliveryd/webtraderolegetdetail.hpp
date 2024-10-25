
#ifndef __GNET_WEBTRADEROLEGETDETAIL_HPP
#define __GNET_WEBTRADEROLEGETDETAIL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "webtrademarket.h"
#include "webtradegetdetail_re.hpp"

namespace GNET
{

class WebTradeRoleGetDetail : public GNET::Protocol
{
	#include "webtraderolegetdetail"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		DEBUG_PRINT("WebTradeRoleGetDetail: receive. userid=%d,roleid=%d,localsid=%d\n",
				userid,roleid,localsid);
		Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
		UserInfo * userinfo = UserContainer::GetInstance().FindUser(userid);
		if( NULL==userinfo || userinfo->logicuid!=LOGICUID(roleid) || !userinfo->rolelist.IsRoleExist(roleid)
				|| !userinfo->CheckCancelSellRole(roleid))
		{
			Log::log(LOG_ERR,"gdelivery::webtraderolegetdetail: user %d role %d invalid", userid, roleid);
			return;
		}
		
		WebTradeGetDetail_Re re;
		re.retcode = ERR_SUCCESS;
		re.localsid = localsid;
		if ( !WebTradeMarket::GetInstance().GetRoleWebTrade(roleid,re.detail) )
			re.retcode = ERR_WT_ENTRY_NOT_FOUND;
		re.sn = re.detail.info.sn;
		//未与平台同步完毕的寄售角色由于数据量过大，不再发给客户端
		if(re.detail.rolebrief.size() > 256)
			re.detail.rolebrief.clear();	
		GDeliveryServer::GetInstance()->Send( userinfo->linksid,re );	
	}
};

};

#endif
