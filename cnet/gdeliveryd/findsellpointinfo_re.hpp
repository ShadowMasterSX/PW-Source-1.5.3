
#ifndef __GNET_FINDSELLPOINTINFO_RE_HPP
#define __GNET_FINDSELLPOINTINFO_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class FindSellPointInfo_Re : public GNET::Protocol
{
	#include "findsellpointinfo_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
	}
};

};

#endif
