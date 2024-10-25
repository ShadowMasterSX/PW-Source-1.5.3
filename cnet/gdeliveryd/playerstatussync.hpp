
#ifndef __GNET_PLAYERSTATUSSYNC_HPP
#define __GNET_PLAYERSTATUSSYNC_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gproviderserver.hpp"
#include "playeroffline.hpp"
#include "mapforbid.h"
#include "mapuser.h"
namespace GNET
{

class PlayerStatusSync : public GNET::Protocol
{
	#include "playerstatussync"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		//用户没有处于踢人或者断线状态，则发送相应协议
		int userid = UidConverter::Instance().Roleid2Uid(roleid);
		if(!userid)
		{
			Log::log(LOG_ERR,"PlayerStatusSync. roleid=%d not found,linksid=%d,localsid=%d,gameid=%d",
					roleid,sid,localsid,gs_id);
			GProviderServer::GetInstance()->DispatchProtocol(gs_id,this);
			return;
		}       
		if (ForbiddenUsers::GetInstance().IsExist(userid))
		{
			//用户处于踢人或断线状态，则丢弃本协议。等待踢人、断线协议处理该用户	
			DEBUG_PRINT("gdelivery::PlayerStatusSync: roleid=%d not found,localsid=%d,gsid=%d\n",roleid,localsid,gs_id);
			return;
		}
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRole(roleid);
		if (!pinfo||pinfo->user->linkid!=provider_link_id||pinfo->localsid!=localsid)
		{
			//向相应gameserver发送statusSync
			GProviderServer::GetInstance()->DispatchProtocol(gs_id,this);
			DEBUG_PRINT("gdelivery::PlayerStatusSync: Send to gameserver. roleid=%d,localsid=%d,gsid=%d\n",roleid,localsid,gs_id);
		}
		else 
		{
                        Log::log(LOG_ERR,"PlayerStatusSync from link,roleid=%d,userid=%d,linksid=%d,localsid=%d,gameid=%d",
                                roleid,userid,sid,localsid,gs_id);
			UserContainer::GetInstance().UserLogout(pinfo->user);
		}
	}
};

};

#endif
