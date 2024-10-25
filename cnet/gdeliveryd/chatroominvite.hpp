
#ifndef __GNET_CHATROOMINVITE_HPP
#define __GNET_CHATROOMINVITE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "chatroominvite_re.hpp"
#include "mapuser.h"
namespace GNET
{

class ChatRoomInvite : public GNET::Protocol
{
	#include "chatroominvite"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(invitee==0)
		{
			UserContainer::GetInstance().FindRoleId(name,invitee);
		}
		int ret = ERR_CHAT_INVALID_ROLE;
		if(invitee && invitee!=inviter)
			ret = RoomManager::GetInstance()->Invite(roomid, invitee, inviter);
		if(ret)
		{
			ChatRoomInvite_Re re;
			re.roomid = roomid;
			re.invitee = invitee;
			re.inviter = inviter;
			re.retcode = ret;
			re.localsid = localsid;
			manager->Send(sid, re);
		}

	}
};

};

#endif
