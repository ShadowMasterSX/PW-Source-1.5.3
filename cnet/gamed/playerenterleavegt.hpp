
#ifndef __GNET_PLAYERENTERLEAVEGT_HPP
#define __GNET_PLAYERENTERLEAVEGT_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void player_enter_leave_gt(int op,int roleid);
namespace GNET
{

class PlayerEnterLeaveGT : public GNET::Protocol
{
	#include "playerenterleavegt"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		player_enter_leave_gt(operation,roleid);
	}
};

};

#endif
