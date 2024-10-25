
#ifndef __GNET_CHATBROADCAST_HPP
#define __GNET_CHATBROADCAST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "worldchat.hpp"
#include "battlechallenge.hpp"
#include "forwardchat.hpp"

namespace GNET
{

class ChatBroadCast : public GNET::Protocol
{
	#include "chatbroadcast"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		LOG_TRACE("ChatBroadCast: challel=%d roleid=%d emotion=%d", channel, srcroleid, emotion);
		GDeliveryServer* dsm = GDeliveryServer::GetInstance();
		WorldChat chat;
		chat.channel = channel;
		chat.roleid  = srcroleid;
		chat.data  = data;
		if(chat.roleid>32 && channel!=GN_CHAT_CHANNEL_SYSTEM){	
			Thread::RWLock::RDScoped l(UserContainer::GetInstance().GetLocker());
			PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline(srcroleid);
			if(NULL == pinfo)
				return;
			chat.name = pinfo->name;
			if(dsm->iweb_sid != _SID_INVALID)
			{
				ForwardChat data(dsm->zoneid,pinfo->gameid,pinfo->userid,srcroleid,pinfo->name,msg);
				dsm->Send(dsm->iweb_sid, data);
			}
		}
		chat.emotion = emotion;
		chat.msg = msg;
		LinkServer::GetInstance().BroadcastProtocol(&chat);
	}
};

};

#endif
