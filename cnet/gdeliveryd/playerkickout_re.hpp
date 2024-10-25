
#ifndef __GNET_PLAYERKICKOUT_RE_HPP
#define __GNET_PLAYERKICKOUT_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "onlineannounce.hpp"
#include "gdeliveryserver.hpp"
#include "kickoutuser.hpp"
#include "mapforbid.h"
#include "mapuser.h"
#include "mapremaintime.h"
#include "kickoutremoteuser_re.hpp"

namespace GNET
{

class PlayerKickout_Re : public GNET::Protocol
{
	#include "playerkickout_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("playerkickout_re: receive playerkickout_re from gs, roleid=%d,result=%d\n",roleid,result);
		int uid = UidConverter::Instance().Roleid2Uid(roleid);
		if(!uid)
			return;
		if (result==ERR_SUCCESS)
			ForbiddenUsers::GetInstance().Pop(uid);
		UserContainer::GetInstance().ContinueLogin(uid, result==ERR_SUCCESS);
		
		//通知原服 踢跨服玩家成功
		if(GDeliveryServer::GetInstance()->IsCentralDS()) {
			UserInfo* pUser = UserContainer::GetInstance().FindUser(uid);
			if(pUser == NULL) return;

			CrossInfoData* pCrsRole = pUser->GetCrossInfo(roleid);
			if(pCrsRole != NULL && pCrsRole->src_zoneid != 0) {
				LOG_TRACE("Tell DS zoneid %d Kickout user %d success", pCrsRole->src_zoneid, uid);
				CentralDeliveryServer::GetInstance()->DispatchProtocol(pCrsRole->src_zoneid, KickoutRemoteUser_Re(result, uid));
			}
		}
	}
};

};

#endif
