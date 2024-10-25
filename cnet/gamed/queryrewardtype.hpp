
#ifndef __GNET_QUERYREWARDTYPE_HPP
#define __GNET_QUERYREWARDTYPE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class QueryRewardType : public GNET::Protocol
{
	#include "queryrewardtype"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
