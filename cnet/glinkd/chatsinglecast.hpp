
#ifndef __GNET_CHATSINGLECAST_HPP
#define __GNET_CHATSINGLECAST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "chatmessage.hpp"
#include "glinkserver.hpp"
namespace GNET
{

class ChatSingleCast : public GNET::Protocol
{
	#include "chatsinglecast"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if (GLinkServer::ValidRole(dstlocalsid,dstroleid))
			GLinkServer::GetInstance()->Send(dstlocalsid,ChatMessage(channel,emotion,srcroleid,msg, data));
	}
};

};

#endif
