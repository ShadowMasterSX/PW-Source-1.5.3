
#ifndef __GNET_KICKOUTREMOTEUSER_HPP
#define __GNET_KICKOUTREMOTEUSER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class KickoutRemoteUser : public GNET::Protocol
{
	#include "kickoutremoteuser"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		LOG_TRACE("Recv KickoutRemoteUser userid %d src_zoneid %d", userid, zoneid);
		if(!GDeliveryServer::GetInstance()->IsCentralDS()) return;
		
		UserInfo* pinfo = UserContainer::GetInstance().FindUser(userid);
		if(pinfo == NULL || pinfo->src_zoneid != zoneid) {
			manager->Send(sid, KickoutRemoteUser_Re(ERR_SUCCESS, userid));
			Log::log(LOG_ERR, "KickoutRemoteUser userid %d, pro.zoneid %d user.src_zoneid %d do not match", userid, zoneid, pinfo == NULL ? -1 : pinfo->src_zoneid);
			return;
		}
		
		/*为了对付工作室账号 采取静默踢下线方式 KickoutUser.reason由 ERR_KICKOUT 改为 0
		  由此可能导致因原服其他原因而致使的踢下线也不会给玩家提示 20120517  */	
		GDeliveryServer::GetInstance()->Send(pinfo->linksid, KickoutUser(userid, pinfo->localsid, 0));
		UserContainer::GetInstance().UserLogout(pinfo, KICKOUT_REMOTE);
	}
};

};

#endif
