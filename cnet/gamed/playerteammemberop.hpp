
#ifndef __GNET_PLAYERTEAMMEMBEROP_HPP
#define __GNET_PLAYERTEAMMEMBEROP_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class PlayerTeamMemberOp : public GNET::Protocol
{
	#include "playerteammemberop"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
