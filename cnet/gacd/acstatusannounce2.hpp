
#ifndef __GNET_ACSTATUSANNOUNCE2_HPP
#define __GNET_ACSTATUSANNOUNCE2_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "aconlinestatus2"

namespace GNET
{

class ACStatusAnnounce2 : public GNET::Protocol
{
	#include "acstatusannounce2"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
