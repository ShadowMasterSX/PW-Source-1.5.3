
#ifndef __GNET_ROLEENTERVOICECHANNELACK_HPP
#define __GNET_ROLEENTERVOICECHANNELACK_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class RoleEnterVoiceChannelAck : public GNET::Protocol
{
	#include "roleentervoicechannelack"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
