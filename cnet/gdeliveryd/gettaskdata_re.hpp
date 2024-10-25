
#ifndef __GNET_GETTASKDATA_RE_HPP
#define __GNET_GETTASKDATA_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class GetTaskData_Re : public GNET::Protocol
{
	#include "gettaskdata_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
