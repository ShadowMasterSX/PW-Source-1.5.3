
#ifndef __GNET_KEKINGNOTIFY_HPP
#define __GNET_KEKINGNOTIFY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class KEKingNotify : public GNET::Protocol
{
	#include "kekingnotify"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
