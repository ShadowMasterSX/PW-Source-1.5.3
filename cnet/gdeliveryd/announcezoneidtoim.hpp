
#ifndef __GNET_ANNOUNCEZONEIDTOIM_HPP
#define __GNET_ANNOUNCEZONEIDTOIM_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class AnnounceZoneidToIM : public GNET::Protocol
{
	#include "announcezoneidtoim"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
