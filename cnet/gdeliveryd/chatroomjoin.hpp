
#ifndef __GNET_CHATROOMJOIN_HPP
#define __GNET_CHATROOMJOIN_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "mapuser.h"
namespace GNET
{

class ChatRoomJoin : public GNET::Protocol
{
	#include "chatroomjoin"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(roomid==0&&ownerid==0)
		{
			UserContainer::GetInstance().FindRoleId( ownername, ownerid );
		}

		ChatRoomJoin_Re re;
		re.retcode = RoomManager::GetInstance()->Join(roomid, ownerid, roleid, password, re.detail);
		re.roleid = roleid;
		re.roomid = roomid;
		re.localsid = localsid;
		GDeliveryServer::GetInstance()->Send(sid, re);
	}
};

};

#endif
