
#ifndef __GNET_MNDOMAINBATTLEOVER_HPP
#define __GNET_MNDOMAINBATTLEOVER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class MNDomainBattleOver : public GNET::Protocol
{
	#include "mndomainbattleover"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
