
#ifndef __GNET_DELFACTIONANNOUNCE_HPP
#define __GNET_DELFACTIONANNOUNCE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class DelFactionAnnounce : public GNET::Protocol
{
	#include "delfactionannounce"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
