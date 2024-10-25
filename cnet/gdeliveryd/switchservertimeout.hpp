
#ifndef __GNET_SWITCHSERVERTIMEOUT_HPP
#define __GNET_SWITCHSERVERTIMEOUT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "mapuser.h"
namespace GNET
{

class SwitchServerTimeout : public GNET::Protocol
{
	#include "switchservertimeout"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		//只处理用户在切换过程中发生断线踢人的情况，正常情况则等待link发送来的断线消息
		Thread::RWLock::WRScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRole((roleid));
		if(!UserContainer::GetInstance().CheckSwitch(pinfo,roleid,link_id,localsid,manager,sid))
			return;
		//用户在切换过程中没有发生踢人或者掉线，则将用户的switch_gs_id设置为空
		pinfo->user->switch_gsid = _GAMESERVER_ID_INVALID;
		DEBUG_PRINT("gdelivery::SwitchServerTimeout. user(r:%d, gs_id:%d).pinfo(%d),pinfo->user(%d)\n",pinfo->roleid,pinfo->gameid,(int)pinfo,(int)pinfo->user);

	}
};

};

#endif
