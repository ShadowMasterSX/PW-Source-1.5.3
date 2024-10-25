
#ifndef __GNET_ROLELEAVEVOICECHANNELACK_HPP
#define __GNET_ROLELEAVEVOICECHANNELACK_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class RoleLeaveVoiceChannelAck : public GNET::Protocol
{
	#include "roleleavevoicechannelack"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
