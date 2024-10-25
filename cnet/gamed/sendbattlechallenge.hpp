
#ifndef __GNET_SENDBATTLECHALLENGE_HPP
#define __GNET_SENDBATTLECHALLENGE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"
#include "groleinventory"
#include "grolestorehouse"
#include "gmailsyncdata"

namespace GNET
{

class SendBattleChallenge : public GNET::Protocol
{
	#include "sendbattlechallenge"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
