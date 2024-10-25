
#ifndef __GNET_TOUCHPOINTQUERY_RE_HPP
#define __GNET_TOUCHPOINTQUERY_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class TouchPointQuery_Re : public GNET::Protocol
{
	#include "touchpointquery_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
