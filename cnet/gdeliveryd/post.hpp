
#ifndef __GNET_POST_HPP
#define __GNET_POST_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "traderinfo"
#include "traderinfo"
#include "timeinfo"
#include "postinfo"

namespace GNET
{

class Post : public GNET::Protocol
{
	#include "post"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
