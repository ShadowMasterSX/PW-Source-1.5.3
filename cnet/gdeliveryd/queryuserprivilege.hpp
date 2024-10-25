
#ifndef __GNET_QUERYUSERPRIVILEGE_HPP
#define __GNET_QUERYUSERPRIVILEGE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class QueryUserPrivilege : public GNET::Protocol
{
	#include "queryuserprivilege"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
