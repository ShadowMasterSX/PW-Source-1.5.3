
#ifndef __GNET_STOCKBILL_RE_HPP
#define __GNET_STOCKBILL_RE_HPP

#include "rpcdefs.h"
#include "callid.hxx"
#include "state.hxx"

namespace GNET
{

class StockBill_Re : public GNET::Protocol
{
	#include "stockbill_re"

	void Process(Manager *manager, Manager::Session::ID sid)
	{
		GLinkServer::GetInstance()->Send(localsid,this);
	}
};

};

#endif
