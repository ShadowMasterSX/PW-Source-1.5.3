
#ifndef __GNET_ADDCASHNOTIFY_HPP
#define __GNET_ADDCASHNOTIFY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class AddCashNotify : public GNET::Protocol
{
	#include "addcashnotify"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
