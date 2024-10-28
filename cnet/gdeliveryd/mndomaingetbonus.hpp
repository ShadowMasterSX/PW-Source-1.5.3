
#ifndef __GNET_MNDOMAINGETBONUS_HPP
#define __GNET_MNDOMAINGETBONUS_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class MNDomainGetBonus : public GNET::Protocol
{
	#include "mndomaingetbonus"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		DEBUG_PRINT("file=%s\tfunc=%s\tline=%d\n", __FILE__, __FUNCTION__, __LINE__);
		// TODO
	}
};

};

#endif
