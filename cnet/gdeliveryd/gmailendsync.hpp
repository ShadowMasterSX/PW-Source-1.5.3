
#ifndef __GNET_GMAILENDSYNC_HPP
#define __GNET_GMAILENDSYNC_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "gshoplog"
#include "gmailsyncdata"

namespace GNET
{

class GMailEndSync : public GNET::Protocol
{
	#include "gmailendsync"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
