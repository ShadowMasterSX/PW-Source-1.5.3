
#ifndef __GNET_CHATROOMSPEAK_HPP
#define __GNET_CHATROOMSPEAK_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "chatroomleave.hpp"

namespace GNET
{

class ChatRoomSpeak : public GNET::Protocol
{
	#include "chatroomspeak"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(RoomManager::GetInstance()->Speak(roomid, message, src, dst))
		{
			ChatRoomLeave re;
			re.roomid = roomid;
			re.roleid = src;
			re.localsid = localsid;
			manager->Send(sid, re);
		}
	}
};

};

#endif
