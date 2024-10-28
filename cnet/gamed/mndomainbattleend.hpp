
#ifndef __GNET_MNDOMAINBATTLEEND_HPP
#define __GNET_MNDOMAINBATTLEEND_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class MNDomainBattleEnd : public GNET::Protocol
{
	#include "mndomainbattleend"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
