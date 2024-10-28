
#ifndef __GNET_MNDOMAINBATTLEENTER_HPP
#define __GNET_MNDOMAINBATTLEENTER_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class MNDomainBattleEnter : public GNET::Protocol
{
	#include "mndomainbattleenter"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
