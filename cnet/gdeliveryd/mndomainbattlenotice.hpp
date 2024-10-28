
#ifndef __GNET_MNDOMAINBATTLENOTICE_HPP
#define __GNET_MNDOMAINBATTLENOTICE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class MNDomainBattleNotice : public GNET::Protocol
{
	#include "mndomainbattlenotice"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
		// TODO
	}
};

};

#endif
