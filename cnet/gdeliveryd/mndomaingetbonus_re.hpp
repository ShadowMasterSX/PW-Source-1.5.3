
#ifndef __GNET_MNDOMAINGETBONUS_RE_HPP
#define __GNET_MNDOMAINGETBONUS_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

#include "mndomainbonusrewarditem"

namespace GNET
{

class MNDomainGetBonus_Re : public GNET::Protocol
{
	#include "mndomaingetbonus_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
		// TODO
	}
};

};

#endif
