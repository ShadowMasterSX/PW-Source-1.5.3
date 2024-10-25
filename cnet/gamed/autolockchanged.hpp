
#ifndef __GNET_AUTOLOCKCHANGED_HPP
#define __GNET_AUTOLOCKCHANGED_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void player_safe_lock_changed(int roleid, int locktime, int max_locktime);

namespace GNET
{

class AutolockChanged : public GNET::Protocol
{
	#include "autolockchanged"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		player_safe_lock_changed(roleid,timeout,locktime);
	}
};

};

#endif
