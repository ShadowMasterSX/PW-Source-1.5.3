
#ifndef __GNET_SENDREFADDBONUS_HPP
#define __GNET_SENDREFADDBONUS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void player_dividend_notify(int role, int dividend);
namespace GNET
{

class SendRefAddBonus : public GNET::Protocol
{
	#include "sendrefaddbonus"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		player_dividend_notify(roleid, bonus);
	}
};

};

#endif
