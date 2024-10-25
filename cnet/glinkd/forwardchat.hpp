
#ifndef __GNET_FORWARDCHAT_HPP
#define __GNET_FORWARDCHAT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class ForwardChat : public GNET::Protocol
{
	#include "forwardchat"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
