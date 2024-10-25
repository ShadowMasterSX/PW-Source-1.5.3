
#ifndef __GNET_SENDTASKREWARD_HPP
#define __GNET_SENDTASKREWARD_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class SendTaskReward : public GNET::Protocol
{
	#include "sendtaskreward"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
