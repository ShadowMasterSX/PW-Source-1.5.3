
#ifndef __GNET_CHATROOMCREATE_HPP
#define __GNET_CHATROOMCREATE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "chatroom.h"
#include "chatroomcreate_re.hpp"

namespace GNET
{

class ChatRoomCreate : public GNET::Protocol
{
	#include "chatroomcreate"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		ChatRoomCreate_Re re;
		re.retcode = RoomManager::GetInstance()->Create(re.roomid,roleid,subject,password,capacity);
		if(password.size()>0)
			re.status = ChatRoom::STATUS_PROTECTED;
		re.subject = subject;
		re.capacity = capacity;
		re.localsid = localsid;
		manager->Send(sid, re);
	}
};

};

#endif
