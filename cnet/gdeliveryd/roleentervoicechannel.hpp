
#ifndef __GNET_ROLEENTERVOICECHANNEL_HPP
#define __GNET_ROLEENTERVOICECHANNEL_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "mapuser.h"
#include "roleentervoicechannelack.hpp"
#include "gametalkmanager.h"
#include "gametalkclient.hpp"

namespace GNET
{

class RoleEnterVoiceChannel : public GNET::Protocol
{
	#include "roleentervoicechannel"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("GameTalkManager: RoleEnterVoiceChannel(%d)",roleid);
		Thread::RWLock::WRScoped l(UserContainer::GetInstance().GetLocker());
		int localroleid = (int)roleid;
		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(localroleid);
		if (NULL == pinfo || pinfo->roleid!=localroleid)
			return;
		GameTalkManager::GetInstance()->SendPlayerEnterLeaveGT(pinfo,GT_ENTER_VOICE_CHANNEL);
		
		RoleEnterVoiceChannelAck ack;
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
