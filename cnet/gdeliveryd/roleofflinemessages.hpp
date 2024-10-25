
#ifndef __GNET_ROLEOFFLINEMESSAGES_HPP
#define __GNET_ROLEOFFLINEMESSAGES_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "rolemsgbean"
#include "mapuser.h"
#include "putmessage.hrp"
#include "gdeliveryserver.hpp"

namespace GNET
{

class RoleOfflineMessages : public GNET::Protocol
{
	#include "roleofflinemessages"

private:
	void _SaveOfflineMessage() {
		RoleMsgBeanVector::iterator it = messages.begin();
		RoleMsgBeanVector::iterator end = messages.end();
		for(; it != end; ++it) {
			if(it->content.size() > GT_CONTENT_MAX_LENGTH) continue;

			Message msg;
			msg.channel = CHANNEL_GAMETALK;
			msg.srcroleid = (int)it->sender;
			msg.dstroleid = (int)receiver;
			msg.src_name.swap(it->sendername);
			msg.msg.swap(it->content);
			PutMessage* rpc = (PutMessage*) Rpc::Call(RPC_PUTMESSAGE, msg);
			GameDBClient::GetInstance()->SendProtocol(rpc);
		}
		GameTalkManager::GetInstance()->SetOfflineMessage((int)receiver);
	}
public:
	void Process(Manager *manager, Manager::Session::ID sid)
	{
		PlayerInfo *pinfo = UserContainer::GetInstance().FindRoleOnline((int)receiver);
		if(!pinfo) {
			_SaveOfflineMessage();
			return;
		}

		GetSavedMsg_Re re;
		re.retcode = ERR_SUCCESS;
		re.roleid = (int)receiver;
		re.localsid = pinfo->localsid;

		RoleMsgBeanVector::iterator it = messages.begin();
		RoleMsgBeanVector::iterator end = messages.end();
		for(; it != end; ++it) {
			if(it->content.size() > GT_CONTENT_MAX_LENGTH) continue;

			Message msg;
			msg.channel = CHANNEL_GAMETALK;
			msg.srcroleid = (int)it->sender;
			msg.dstroleid = (int)receiver;
			msg.src_name.swap(it->sendername);
			msg.msg.swap(it->content);
			re.messages.push_back(msg);
		}
		GDeliveryServer::GetInstance()->Send(pinfo->linksid, re);
	}
};

};

#endif
