
#ifndef __GNET_TOUCHPOINTCOST_RE_HPP
#define __GNET_TOUCHPOINTCOST_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

void OnTouchPointCost(int roleid,int64_t orderid,unsigned int cost,int64_t income,int64_t remain,int retcode);	

namespace GNET
{

class TouchPointCost_Re : public GNET::Protocol
{
	#include "touchpointcost_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		// TODO
		OnTouchPointCost(roleid,orderid,cost,income,remain,retcode);
	}
};

};

#endif
