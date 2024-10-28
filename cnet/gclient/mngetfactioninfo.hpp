
#ifndef __GNET_MNGETFACTIONINFO_HPP
#define __GNET_MNGETFACTIONINFO_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class MNGetFactionInfo : public GNET::Protocol
{
	#include "mngetfactioninfo"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
