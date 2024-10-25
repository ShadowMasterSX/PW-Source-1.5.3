
#ifndef __GNET_SENDREWARDADDBONUS_HPP
#define __GNET_SENDREWARDADDBONUS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class SendRewardAddBonus : public GNET::Protocol
{
	#include "sendrewardaddbonus"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
