
#ifndef __GNET_VERIFYMASTER_RE_HPP
#define __GNET_VERIFYMASTER_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class VerifyMaster_Re : public GNET::Protocol
{
	#include "verifymaster_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
