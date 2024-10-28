
#ifndef __GNET_CROSSCHAT_RE_HPP
#define __GNET_CROSSCHAT_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "crosssystem.h"

namespace GNET
{

class CrossChat_Re : public GNET::Protocol
{
	#include "crosschat_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		CrossChatClient::GetInstance()->OnRecv(*this);
	}
};

};

#endif
