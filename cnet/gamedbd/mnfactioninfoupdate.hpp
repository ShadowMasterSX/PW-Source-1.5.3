
#ifndef __GNET_MNFACTIONINFOUPDATE_HPP
#define __GNET_MNFACTIONINFOUPDATE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "mnfactioninfo"

namespace GNET
{

class MNFactionInfoUpdate : public GNET::Protocol
{
	#include "mnfactioninfoupdate"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
