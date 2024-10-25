
#ifndef __GNET_NOTIFYFACTIONPLAYERRENAME_HPP
#define __GNET_NOTIFYFACTIONPLAYERRENAME_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class NotifyFactionPlayerRename : public GNET::Protocol
{
	#include "notifyfactionplayerrename"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
