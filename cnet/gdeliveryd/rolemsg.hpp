
#ifndef __GNET_ROLEMSG_HPP
#define __GNET_ROLEMSG_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "rolemsgbean"
#include "mapuser.h"
#include "privatechat.hpp"
#include "gdeliveryserver.hpp"
#include "gametalkmanager.h"
#include "gametalkdefs.h"

namespace GNET
{

class RoleMsg : public GNET::Protocol
{
	#include "rolemsg"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(message.content.size() > GT_CONTENT_MAX_LENGTH) return;

		PlayerInfo * pinfo = UserContainer::GetInstance().FindRoleOnline((int)receiver);
		if(pinfo) {
			PrivateChat chat;
			chat.dstroleid = (int)receiver;
			chat.channel = CHANNEL_GAMETALK;
			chat.emotion = message.emotiongroup;
			chat.src_name.swap(message.sendername);
			chat.srcroleid = (int)message.sender;
			chat.msg.swap(message.content);
			GDeliveryServer::GetInstance()->Send(pinfo->linksid, chat);
		} else {
			Message msg;
			msg.channel = CHANNEL_GAMETALK;
			msg.srcroleid = (int)message.sender;
			msg.dstroleid = (int)receiver;
			msg.src_name.swap(message.sendername);
			msg.msg.swap(message.content);
			PutMessage* rpc = (PutMessage*) Rpc::Call(RPC_PUTMESSAGE, msg);
			GameDBClient::GetInstance()->SendProtocol(rpc);
			GameTalkManager::GetInstance()->SetOfflineMessage(msg.dstroleid);
		}
	}
};

};

#endif
