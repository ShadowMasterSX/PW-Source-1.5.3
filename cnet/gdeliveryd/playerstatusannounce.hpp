
#ifndef __GNET_PLAYERSTATUSANNOUNCE_HPP
#define __GNET_PLAYERSTATUSANNOUNCE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include <onlineplayerstatus>
namespace GNET
{

class PlayerStatusAnnounce : public GNET::Protocol
{
	#include "playerstatusannounce"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
