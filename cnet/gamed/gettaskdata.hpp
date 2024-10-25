
#ifndef __GNET_GETTASKDATA_HPP
#define __GNET_GETTASKDATA_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class GetTaskData : public GNET::Protocol
{
	#include "gettaskdata"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
