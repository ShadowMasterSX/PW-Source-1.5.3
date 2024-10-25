
#ifndef __GNET_ANNOUNCEAUTHDVERSION_HPP
#define __GNET_ANNOUNCEAUTHDVERSION_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class AnnounceAuthdVersion : public GNET::Protocol
{
	#include "announceauthdversion"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
