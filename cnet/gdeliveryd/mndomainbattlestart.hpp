
#ifndef __GNET_MNDOMAINBATTLESTART_HPP
#define __GNET_MNDOMAINBATTLESTART_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class MNDomainBattleStart : public GNET::Protocol
{
	#include "mndomainbattlestart"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
	}
};

};

#endif
