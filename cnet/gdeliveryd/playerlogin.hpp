
#ifndef __GNET_PLAYERLOGIN_HPP
#define __GNET_PLAYERLOGIN_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "localmacro.h"
#include "gdeliveryserver.hpp"
#include "mapuser.h"
#include "crosssystem.h"

namespace GNET
{

class PlayerLogin : public GNET::Protocol
{
	#include "playerlogin"
	void SendFailResult(GDeliveryServer* dsm,Manager::Session::ID sid,int retcode);
	void SendForbidInfo(GDeliveryServer* dsm,Manager::Session::ID sid,const GRoleForbid& forbid);
	bool PermitLogin(GDeliveryServer* dsm,Manager::Session::ID sid);
	void SetGMPrivilege( int roleid );
	void DoLogin(Manager::Session::ID sid, UserInfo* pUser, bool is_central);
	void TryRemoteLogin(Manager::Session::ID sid, UserInfo* pUser);

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("gdelivery::receive playerlogin from link,roleid=%d,linkid=%d,localsid=%d\n",
			roleid,provider_link_id,localsid);
		GDeliveryServer* dsm = GDeliveryServer::GetInstance();
		
		int userid = UidConverter::Instance().Roleid2Uid(roleid);
		UserInfo* pUser = UserContainer::GetInstance().FindUser(userid);
		if( (pUser == NULL) || (pUser->localsid != localsid) || (pUser->status != _STATUS_ONLINE) || (pUser->logicuid != LOGICUID(roleid)) || !pUser->rolelist.IsRoleExist(roleid) ) {
			SendFailResult(dsm, sid, ERR_LOGINFAIL);
			return;
		}

		bool is_central = dsm->IsCentralDS();
		bool is_cross_locked = pUser->IsRoleCrossLocked(roleid);
		
		if ((is_central || is_cross_locked) && pUser->is_phone)
		{
			// 使用手机登录跨服服务器或者处于跨服锁定状态的角色
			SendFailResult(dsm, sid, ERR_LOGINFAIL);
			return;
		}

		//flag 0:正常登陆逻辑 1:原服到跨服 2:跨服到原服 3:直接登录跨服服务器
		if(is_central) //跨服收到此协议
		{
			//分为从原服->跨服 flag == DS_TO_CENTRALDS，和直接登录跨服 flag == DIRECT_TO_CENTRALDS 两种情况
			if(flag != DS_TO_CENTRALDS && flag != DIRECT_TO_CENTRALDS) return;
		}
		else //原服收到此协议
		{
			//分为正常登陆原服flag == 0，和从跨服回到原服flag == CENTRALDS_TO_DS
			if(flag != CENTRALDS_TO_DS && flag != 0) return;
			//尝试进行跨服直接登陆
			if(flag == 0 && is_cross_locked) flag = DIRECT_TO_CENTRALDS;
		}
	
		if(is_central) {
			if( !PermitLogin(dsm,sid) ) return;
			//old version
			DoLogin(sid, pUser, is_central);
		} else {
			if(flag == DIRECT_TO_CENTRALDS) {
				TryRemoteLogin(sid, pUser);
			} else {
				if( !PermitLogin(dsm,sid) ) return;
				//old version
				DoLogin(sid, pUser, is_central);
			}
		}
	}
};

};

#endif
