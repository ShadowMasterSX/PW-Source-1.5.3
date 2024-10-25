
#ifndef __GNET_ACSTATUSANNOUNCE_HPP
#define __GNET_ACSTATUSANNOUNCE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "aconlinestatus"
namespace GNET
{

class ACStatusAnnounce : public GNET::Protocol
{
	#include "acstatusannounce"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
