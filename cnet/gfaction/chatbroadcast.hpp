
#ifndef __GNET_CHATBROADCAST_HPP
#define __GNET_CHATBROADCAST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "worldchat.hpp"
#include "factiondb.h"

namespace GNET
{

class ChatBroadCast : public GNET::Protocol
{
	#include "chatbroadcast"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("ChatBroadCast type=%d fid=%d\n",channel, srcroleid);
		WorldChat chat;
		chat.msg = msg;
		chat.channel = GN_CHAT_CHANNEL_SYSTEM;
		chat.roleid = channel;
		Factiondb::GetInstance()->FindFactionName(srcroleid, chat.name);
		GFactionServer::GetInstance()->BroadcastLink(chat);
	}
};

};

#endif
