
#ifndef __GNET_QUERYUSERFORBID_HPP
#define __GNET_QUERYUSERFORBID_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class QueryUserForbid : public GNET::Protocol
{
	#include "queryuserforbid"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
