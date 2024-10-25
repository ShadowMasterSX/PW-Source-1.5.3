
#ifndef __GNET_REPORTIP_HPP
#define __GNET_REPORTIP_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class ReportIP : public GNET::Protocol
{
	#include "reportip"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
