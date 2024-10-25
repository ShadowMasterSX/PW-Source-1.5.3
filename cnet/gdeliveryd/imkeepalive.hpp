
#ifndef __GNET_IMKEEPALIVE_HPP
#define __GNET_IMKEEPALIVE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gametalkmanager.h"
#include "gametalkclient.hpp"
#include "snsclient.hpp"

namespace GNET
{

class IMKeepAlive : public GNET::Protocol
{
	#include "imkeepalive"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		if(manager == GameTalkClient::GetInstance())
		{
			manager->Send(sid, this);
			GameTalkManager::GetInstance()->FlushBuffer();
		}
		else if(manager == SNSClient::GetInstance())
		{
			manager->Send(sid, this);
		}
	}
};

};

#endif
