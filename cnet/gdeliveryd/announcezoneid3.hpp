
#ifndef __GNET_ANNOUNCEZONEID3_HPP
#define __GNET_ANNOUNCEZONEID3_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class AnnounceZoneid3 : public GNET::Protocol
{
	#include "announcezoneid3"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
