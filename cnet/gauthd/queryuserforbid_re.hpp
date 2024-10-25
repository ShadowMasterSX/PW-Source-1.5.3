
#ifndef __GNET_QUERYUSERFORBID_RE_HPP
#define __GNET_QUERYUSERFORBID_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class QueryUserForbid_Re : public GNET::Protocol
{
	#include "queryuserforbid_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
