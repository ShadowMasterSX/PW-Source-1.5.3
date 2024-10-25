
#ifndef __GNET_ADDCASHNOTIFY_HPP
#define __GNET_ADDCASHNOTIFY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void player_add_cash_notify(int roleid);

namespace GNET
{

class AddCashNotify : public GNET::Protocol
{
	#include "addcashnotify"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		player_add_cash_notify(roleid);
	}
};

};

#endif
