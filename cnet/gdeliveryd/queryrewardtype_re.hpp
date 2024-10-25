
#ifndef __GNET_QUERYREWARDTYPE_RE_HPP
#define __GNET_QUERYREWARDTYPE_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class QueryRewardType_Re : public GNET::Protocol
{
	#include "queryrewardtype_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
