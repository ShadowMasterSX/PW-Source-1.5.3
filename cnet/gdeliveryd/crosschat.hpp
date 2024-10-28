
#ifndef __GNET_CROSSCHAT_HPP
#define __GNET_CROSSCHAT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "crosssystem.h"

namespace GNET
{

class CrossChat : public GNET::Protocol
{
	#include "crosschat"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		CrossChatServer::GetInstance()->OnRecv(*this);
	}
};

};

#endif
