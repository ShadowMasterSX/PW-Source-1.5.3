
#ifndef __GNET_MNDOMAINBATTLESTART_RE_HPP
#define __GNET_MNDOMAINBATTLESTART_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class MNDomainBattleStart_Re : public GNET::Protocol
{
	#include "mndomainbattlestart_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\tretcode=%d\tdomain_id=%d\tworld_tag=%d\n", __FILE__, __FUNCTION__, __LINE__, retcode, domain_id, world_tag);
	}
};

};

#endif
