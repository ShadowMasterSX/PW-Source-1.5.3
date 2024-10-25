
#ifndef __GNET_SETTASKDATA_HPP
#define __GNET_SETTASKDATA_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class SetTaskData : public GNET::Protocol
{
	#include "settaskdata"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
