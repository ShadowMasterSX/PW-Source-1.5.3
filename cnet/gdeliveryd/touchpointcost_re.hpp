
#ifndef __GNET_TOUCHPOINTCOST_RE_HPP
#define __GNET_TOUCHPOINTCOST_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"


namespace GNET
{

class TouchPointCost_Re : public GNET::Protocol
{
	#include "touchpointcost_re"
	enum
	{
		TPC_SUCCESS,
		TPC_ORDER_CLASH,
		TPC_NEED_MORE,
		TPC_COMPLETE,
		TPC_NEED_GOLD,
		TPC_BUSY,
		TPC_NO_ACCOUNT,
		TPC_OTHER,

		TPC_FAIL = 255
	};

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
	}
};

};

#endif
