
#ifndef __GNET_ENTERWORLD_HPP
#define __GNET_ENTERWORLD_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gdeliveryserver.hpp"
#include "gproviderserver.hpp"

#include "gamedbclient.hpp"

#include "gproviderserver.hpp"
#include "playerstatusannounce.hpp"
#include "acstatusannounce2.hpp"
#include "gfactionclient.hpp"
#include "ganticheatclient.hpp"
#include "mapuser.h"
#include "setchatemotion.hpp"
#include "addictioncontrol.hpp"
#include "referencemanager.h"
#include "rewardmanager.h"
#include "gametalkmanager.h"
#include "friendextinfomanager.h"
#include "disabled_system.h"
#include "kingelection.h"

namespace GNET
{

class EnterWorld : public GNET::Protocol
{
	#include "enterworld"
	void AnnouncePlayerOnline(UserInfo& ui)
	{
		PlayerStatusAnnounce psa;
		psa.status=_STATUS_ONGAME;
		psa.playerstatus_list.add(OnlinePlayerStatus(ui.roleid,ui.gameid,ui.linkid,ui.localsid));
		GProviderServer::GetInstance()->BroadcastProtocol(psa);
		GFactionClient::GetInstance()->SendProtocol(psa);

		ACStatusAnnounce2 acsa;
		acsa.status=_STATUS_ONGAME;
		if (ui.is_phone)
			acsa.status |= AC_LOGIN_STATUS_MOBILE;
		acsa.info_list.push_back( ACOnlineStatus2(ui.roleid, ui.userid, ui.ip) );
		GAntiCheatClient::GetInstance()->SendProtocol(acsa);
	}
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		Thread::RWLock::WRScoped l(UserContainer::GetInstance().GetLocker());
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRole(roleid);
		if (NULL==pinfo) return;
		if (pinfo->linksid!=sid || pinfo->localsid!=localsid )
			return;
		UserInfo* user = pinfo->user;
		if(user->status!=_STATUS_READYGAME)
			return;
		user->status=_STATUS_ONGAME;
		pinfo->ingame = true;
		user->GetLocktime(locktime, timeout, settime);

		GProviderServer::GetInstance()->DispatchProtocol(pinfo->gameid,this);
		int now = Timer::GetTime();
		FriendextinfoManager::GetInstance()->UpdateRoleLoginTime(pinfo,now);
		//Announce Player Online
		AnnouncePlayerOnline(*user);

		if(pinfo->emotion)
		{
			// 帮派服务器收到PlayerStatusAnnounce才会创建玩家数据,所以现在才发emotion设置
			GFactionClient::GetInstance()->SendProtocol(SetChatEmotion(roleid, pinfo->emotion));
		}
		if(user->actime>0)
		{
			for(std::vector<GPair>::iterator it=user->acstate->data.begin(),ie=user->acstate->data.end();it!=ie;++it)
			{
				if(it->key==1)
					it->value += Timer::GetTime() - user->actime;
			}
			user->acstate->userid = pinfo->roleid;
			GProviderServer::GetInstance()->DispatchProtocol(user->gameid,user->acstate);
			user->actime = Timer::GetTime();
		}
		ReferenceManager::GetInstance()->OnLogin(user);
		if(!DisabledSystem::GetDisabled(SYS_REWARD)) RewardManager::GetInstance()->OnLogin(roleid, user->userid);
		if(!DisabledSystem::GetDisabled(SYS_PLAYERPROFILE)) PlayerProfileMan::GetInstance()->OnPlayerLogin(roleid);
		KingElection::GetInstance().OnLogin(*pinfo);
	}
};

};

#endif
