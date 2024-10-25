
#ifndef __GNET_SETTASKDATA_RE_HPP
#define __GNET_SETTASKDATA_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class SetTaskData_Re : public GNET::Protocol
{
	#include "settaskdata_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
