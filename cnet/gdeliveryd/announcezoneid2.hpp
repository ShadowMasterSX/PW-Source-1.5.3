
#ifndef __GNET_ANNOUNCEZONEID2_HPP
#define __GNET_ANNOUNCEZONEID2_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class AnnounceZoneid2 : public GNET::Protocol
{
	#include "announcezoneid2"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
