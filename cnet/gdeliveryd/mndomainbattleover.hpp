
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
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
		// TODO
	}
};

};

#endif
