
#ifndef __GNET_FACTIONMSG_HPP
#define __GNET_FACTIONMSG_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "factionidbean"
#include "rolemsgbean"
#include "factionchat.hpp"
#include "gametalkclient.hpp"
#include "gfactionclient.hpp"

namespace GNET
{

class FactionMsg : public GNET::Protocol
{
	#include "factionmsg"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(manager == GFactionClient::GetInstance())
			GameTalkClient::GetInstance()->SendProtocol(this);
		else if(manager == GameTalkClient::GetInstance()) {
			if(message.content.size() > GT_CONTENT_MAX_LENGTH) return;
			FactionChat chat;
			chat.channel = CHANNEL_GAMETALK;
			chat.dst_localsid = factionid.factionid;
			chat.emotion = message.emotiongroup;
			chat.src_roleid = (int)message.sender;
			chat.msg.swap(message.content);
			GFactionClient::GetInstance()->SendProtocol(chat);
		}
	}
};

};

#endif
