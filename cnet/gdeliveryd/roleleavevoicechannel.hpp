
#ifndef __GNET_ROLELEAVEVOICECHANNEL_HPP
#define __GNET_ROLELEAVEVOICECHANNEL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "mapuser.h"
#include "roleleavevoicechannelack.hpp"
#include "gametalkmanager.h"
#include "gametalkclient.hpp"

namespace GNET
{

class RoleLeaveVoiceChannel : public GNET::Protocol
{
	#include "roleleavevoicechannel"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("GameTalkManager: RoleLeaveVoiceChannel(%d)",roleid);
		Thread::RWLock::WRScoped l(UserContainer::GetInstance().GetLocker());
		int localroleid = (int)roleid;
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(localroleid);
		if (NULL == pinfo || pinfo->roleid!=localroleid)
			return;
		GameTalkManager::GetInstance()->SendPlayerEnterLeaveGT(pinfo,GT_LEAVE_VOICE_CHANNEL);
		
		RoleLeaveVoiceChannelAck ack;
		ack.userid = userid;
		ack.roleid = roleid;
		ack.zoneid = zoneid;
		ack.seq = seq;
		ack.timestamp = timestamp;

		GameTalkClient::GetInstance()->SendProtocol(ack);
	}
};

};

#endif
