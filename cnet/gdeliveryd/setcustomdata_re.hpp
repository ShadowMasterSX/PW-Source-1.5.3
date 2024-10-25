
#ifndef __GNET_SETCUSTOMDATA_RE_HPP
#define __GNET_SETCUSTOMDATA_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class SetCustomData_Re : public GNET::Protocol
{
	#include "setcustomdata_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
