
#ifndef __GNET_MNFACTIONAPPLY_HPP
#define __GNET_MNFACTIONAPPLY_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "gmailsyncdata"

namespace GNET
{

class MNFactionApply : public GNET::Protocol
{
	#include "mnfactionapply"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
