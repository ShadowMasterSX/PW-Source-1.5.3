
#ifndef __GNET_NEWKEEPALIVE_HPP
#define __GNET_NEWKEEPALIVE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class NewKeepAlive : public GNET::Protocol
{
	#include "newkeepalive"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
