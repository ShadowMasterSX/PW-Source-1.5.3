
#ifndef __GNET_CROSSGUARDNOTIFY_HPP
#define __GNET_CROSSGUARDNOTIFY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "crosssystem.h"

namespace GNET
{

class CrossGuardNotify : public GNET::Protocol
{
	#include "crossguardnotify"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		CrossGuardClient::GetInstance()->OnUpdate(carnival);
	}
};

};

#endif
